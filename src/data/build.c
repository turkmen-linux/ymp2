#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <libgen.h>
#include <errno.h>
#include <sched.h>

#include <core/ymp.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>
#include <utils/fetcher.h>
#include <utils/archive.h>
#include <utils/yaml.h>

#include <data/build.h>
#include <config.h>

#define UNSHARE_FLAGS (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNET)

visible char* ympbuild_get_value(ympbuild* ymp, const char* name) {
    char* command = build_string(
    "exec <&-\n"
    "{\n%s\n} &>/dev/null\n"
    "echo -n ${%s}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput_unshare(args, UNSHARE_FLAGS | CLONE_NEWPID));
    debug("variable: %s -> %s\n", name, output);
    free(command);
    return output;
}

visible char** ympbuild_get_array(ympbuild* ymp, const char* name){
    char* command = build_string(
    "exec <&-\n"
    "{\n%s\n} &>/dev/null\n"
    "echo -n ${%s[@]}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput_unshare(args, UNSHARE_FLAGS | CLONE_NEWPID));
    debug("variable: %s -> %s\n", name, output);
    free(command);
    return split(output," ");

}

visible int ympbuild_run_function(ympbuild* ymp, const char* name) {
    char* command = build_string(
        "exec <&-\n"
        "set +e ; %s\n"
        "%s\n"
        "set -e \n"
        "if declare -F %s ; then\n"
        "    %s\n"
        "fi", ymp->header, ymp->ctx, name, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    pid_t pid = fork();
    if(pid == 0){
        if(chdir(ymp->path) < 0){
            free(command);
            return -1;
        }
        char* envs[] = {
            build_string("PATH=/usr/bin:/usr/sbin:/bin:/sbin/%s", ymp->path),
            build_string("HOME=%s", ymp->path),
            NULL
        };
        if (unshare(UNSHARE_FLAGS) < 0) {
            exit(1);
        }
        if(sethostname("sandbox",7) < 0){
            exit(1);
        }
        execve(args[0], args, envs);
        free(command);
        return -1;
    } else {
        int status = 0;
        (void)waitpid(pid, &status, 0);
        free(command);
        return status;
    }
}

static char* hash_types[] = {"sha512sums", "sha256sums", "sha1sums", "md5sums", NULL};

static bool get_resource(const char* resource_path, const char* resource_name, size_t resource_type, const char* source_url, const char* expected_hash) {
    debug("Source: %s %s\n", source_url, expected_hash);

    // Get the file name from the source URL
    char* source_file_name = basename((char*)source_url);

    // Construct the target cache directory path
    char* cache_directory = build_string("%s/cache/%s", BUILD_DIR, resource_name);
    char* target_file_path = build_string("%s/%s", cache_directory, source_file_name);

    bool operation_status = true;

    // Check if the target file already exists
    if (!isfile(target_file_path)) {
        // Download or Copy the resource
        create_dir(cache_directory);

        char* local_file_path = build_string("%s/%s", resource_path, source_url);

        if (isfile(local_file_path)) {
            free(target_file_path);
            target_file_path = build_string("%s/%s", cache_directory, source_file_name);
            operation_status = copy_file(local_file_path, target_file_path);
        } else {
            operation_status = fetch(source_url, target_file_path);
        }

        free(local_file_path);
    }

    // Check the hash of the downloaded or copied file
    char* actual_hash = calculate_hash(resource_type, target_file_path);

    if (iseq((char*)expected_hash, "SKIP")) {
        warning("Hash control disabled for: %s\n", source_file_name);
    } else if (!iseq(actual_hash, (char*)expected_hash)) {
        print("Archive hash is invalid:\n  -> Expected: %s\n  -> Received: %s\n", expected_hash, actual_hash);
        return false;
    }

    // Cleanup
    free(cache_directory);
    free(target_file_path);

    return operation_status;
}


static char* actions[] = {"prepare", "setup", "build", "package", NULL};

static char** get_uses(ympbuild *ymp){
    char* uses = variable_get_value(global->variables, "build:use");
    array *flag = array_new();
    if(strlen(uses) > 0){
        array_adds(flag, split(uses, " "));
    } else {
        array_add(flag, "all");
    }
    if(array_has(flag, "all")){
        array_remove(flag, "all");
        array_adds(flag, ympbuild_get_array(ymp, "uses"));
    }
    if(array_has(flag, "extra")){
        array_remove(flag, "extra");
        array_adds(flag, ympbuild_get_array(ymp, "uses_extra"));
    }
    size_t len=0;
    char** ret = array_get(flag, &len);
    array_unref(flag);
    return ret;
}

static void configure_header(ympbuild *ymp) {
    ymp->header = readfile(":/ympbuild-header.sh");
    ymp->header = str_replace(ymp->header, "@buildpath@", ymp->path);
    ymp->header = str_replace(ymp->header, "@CC@", variable_get_value(global->variables, "build:cc"));
    ymp->header = str_replace(ymp->header, "@CXX@", variable_get_value(global->variables, "build:cxx"));
    ymp->header = str_replace(ymp->header, "@CFLAGS@", variable_get_value(global->variables, "build:cflags"));
    ymp->header = str_replace(ymp->header, "@CXXFLAGS@", variable_get_value(global->variables, "build:cxxflags"));
    ymp->header = str_replace(ymp->header, "@LDFLAGS@", variable_get_value(global->variables, "build:ldflags"));
    ymp->header = str_replace(ymp->header, "@APIKEY@", variable_get_value(global->variables, "build:token"));
    ymp->header = str_replace(ymp->header, "@DISTRODIR@", DISTRODIR);
    char** flag = get_uses(ymp);
    for(size_t i=0; flag[i];i++){
        char* new_header = build_string("%s\ndeclare -r use_%s=31\n", ymp->header, flag[i]);
        free(ymp->header);
        ymp->header = new_header;
    }
    free(flag);
}

static void generate_links_files(const char* path){
    char* rootfs = build_string("%s/output", path);
    char** inodes = find(rootfs);
    array *files = array_new();
    array *links = array_new();
    for(size_t i=0; inodes[i]; i++){
        if(issymlink(inodes[i])){
            debug("add symlink: %s\n", inodes[i]);
            array_add(links, build_string("%s %s\n",sreadlink(inodes[i]), inodes[i]+strlen(rootfs)+1));
        } else if(isfile(inodes[i])){
            debug("add file: %s\n", inodes[i]);
            char* hash = calculate_sha1(inodes[i]);
            array_add(files, build_string("%s %s\n", hash, inodes[i]+strlen(rootfs)+1));
            free(hash);
        }
        free(inodes[i]);
    }
    char* files_path = build_string("%s/files", path);
    char* links_path = build_string("%s/links", path);
    writefile(files_path, array_get_string(files));
    writefile(links_path, array_get_string(links));
    // cleanup
    free(inodes);
    free(links_path);
    free(files_path);
    array_unref(files);
    array_unref(links);
}
static char* metadata_vars[] = {"name", "version", "description", "release", NULL};
static char* source_arrs[] = {"depends", "makedepends", "arch", "provides", "replaces", "source", NULL};

static char* getArch() {
    struct utsname buffer;
    errno = 0;
    if (uname(&buffer) < 0) {
        perror("uname");
    }
    return strdup(buffer.machine);
}

static void generate_metadata(ympbuild *ymp, bool is_source) {
    array *a = array_new();
    array_add(a, "ymp:\n");
    if(is_source){
        array_add(a, "  source:\n");
    } else {
        array_add(a, "  package:\n");
    }
    // Common variables
    for(size_t i=0; metadata_vars[i]; i++) {
        array_add(a, build_string("    %s: %s\n",metadata_vars[i], ympbuild_get_value(ymp, metadata_vars[i])));
    }
    // Package arrays
    if(!is_source){
        array_add(a, build_string("    arch: %s\n", getArch()));
        // Dependencies
        array *deps = array_new();
        array_adds(deps, ympbuild_get_array(ymp, "depends"));
        char** flag = get_uses(ymp);
        for(size_t i=0; flag[i]; i++) {
            array_adds(deps, ympbuild_get_array(ymp, build_string("%s_depends", flag[i])));
        }
        array_add(a, "    depends:\n");
        size_t len = 0;
        char** depends = array_get(deps, &len);
        for(size_t i=0; depends[i] && strlen(depends[i]) > 0; i++){
            array_add(a, build_string("      - %s\n", depends[i]));
        }
        free(depends);
        free(flag);
    } else {
        // Source arrays
        for(size_t i=0; source_arrs[i]; i++) {
            char** items = ympbuild_get_array(ymp, source_arrs[i]);
            if(items[0] && strlen(items[0]) > 0){
                array_add(a, build_string("    %s:\n", source_arrs[i]));
                for(size_t j=0; items[j]; j++) {
                    array_add(a, build_string("      - %s\n", items[j]));
                }
            }
        }
        // Use flags
        array *uses = array_new();
        array_adds(uses, ympbuild_get_array(ymp, "uses"));
        array_adds(uses, ympbuild_get_array(ymp, "uses_extra"));
        size_t len = 0;
        char** flags = array_get(uses, &len);
        if(strlen(flags[0]) > 0){
            array_add(a, "    use-flags:\n");
        }
        for(size_t i=0; flags[i] && strlen(flags[i]) > 0; i++){
            array_add(a, build_string("      - %s:\n", flags[i]));
        }
        // Use flag dependencies
        for(size_t i=0; flags[i] && strlen(flags[i]) > 0; i++){
            array_add(a, build_string("    %s-depends:\n", flags[i]));
            char** deps = ympbuild_get_array(ymp, build_string("%s_depends", flags[i]));
            for(size_t j=0; deps[j]; j++){
                array_add(a, build_string("      - %s\n", deps[j]));
                free(deps[j]);
            }
            free(deps);
            free(flags[i]);
        }
        free(flags);
        array_unref(uses);
    }

    char* ret = array_get_string(a);
    array_unref(a);
    writefile(build_string("%s/metadata.yaml", ymp->path), ret);
}

visible char* build_source_from_path(const char* path){
    if(!global){
        print("Error: ymp global missing!\n");
        return NULL;
    }
    char* ympfile = build_string("%s/ympbuild",path);
    if(!isfile(ympfile)){
        free(ympfile);
        return NULL;
    }
    ympbuild *ymp = malloc(sizeof(ympbuild));
    ymp->ctx = readfile(ympfile);
    // define variables
    char* name = ympbuild_get_value(ymp, "name");
    char* version = ympbuild_get_value(ymp, "version");
    char* src_cache = build_string("%s/cache/%s-%s/",BUILD_DIR, name, version);
    create_dir(src_cache);
    // Generate source metadata
    ymp->path = src_cache;
    generate_metadata(ymp, true);
    // detect hash
    char** hashs = NULL;
    size_t hash_type = 0;
    for(hash_type=0; hash_types[hash_type]; hash_type++){
        hashs = ympbuild_get_array(ymp, hash_types[hash_type]);
        if(strlen(hashs[0]) > 0){
            break;
        }
        if(hash_type > 1){
            warning("Weak hash algorithm (%s) detected!.\n", hash_types[hash_type]);
        }
        free(hashs);
    }
    // copy ympbuild file
    char* target = build_string("%s/ympbuild",src_cache);
    copy_file(ympfile, target);
    free(target);
    // copy resources
    char** sources = ympbuild_get_array(ymp, "source");
    for(size_t i=0; sources[i] && hashs[i]; i++){
        if(!get_resource(path, build_string("%s-%s", name, version), hash_type, sources[i], hashs[i])){
            return NULL;
        }
    }
    free(ymp->ctx);
    free(ymp);
    return src_cache;

}

visible char *build_binary_from_path(const char* path){
    if(!global){
        print("Error: ymp global missing!\n");
        return NULL;
    }
    char* ympfile = build_string("%s/ympbuild",path);
    if(!isfile(ympfile)){
        free(ympfile);
        return NULL;
    }
    ympbuild *ymp = malloc(sizeof(ympbuild));
    ymp->ctx = readfile(ympfile);
    // Create build path
    ymp->path = calculate_md5(ympfile);
    ymp->path = build_string("%s/%s", BUILD_DIR, ymp->path);
    create_dir(ymp->path);
    // Configure header
    configure_header(ymp);
    char** src_files = find(path);
    Archive *a = archive_new();
    for(size_t i=0; src_files[i];i++){
        debug("Copy / Extract %s\n",src_files[i]);
        if(archive_is_archive(a, src_files[i])){
            archive_load(a, src_files[i]);
            archive_set_target(a, ymp->path);
            archive_extract_all(a);
        } else {
            char* src_name = src_files[i]+strlen(path);
            char* target_path = build_string("%s/%s", ymp->path, src_name);
            copy_file(src_files[i], target_path);
            free(target_path);
        }
    }
    int status = 0;
    for(size_t i=0; actions[i]; i++){
        status = ympbuild_run_function(ymp, actions[i]);
        if(status != 0){
            free(ympfile);
            return NULL;
        }
    }
    // Generate links file
    generate_links_files(ymp->path);
    generate_metadata(ymp, false);
    char* ret = strdup(ymp->path);
    // Cleanup
    free(ymp);
    free(ympfile);
    return ret;
}

visible bool build_from_path(const char* path){
    char* cache = build_source_from_path(path);
    print("Source created at: %s\n", cache);
    char* build = build_binary_from_path(cache);
    print("Binary created at: %s\n", build);
    return (build != NULL && cache != NULL);
}

visible char* create_package(const char* path){
    char* curdir = pwd();
    char* metadata_file = build_string("%s/metadata.yaml", path);
    char* ret = build_string("%s/package.zip", path);
    char* metadata = readfile(metadata_file);
    if(!isfile(metadata_file)){
        print("Failed to find %s\n", metadata_file);
        return NULL;
    }
    free(metadata_file);
    if(chdir(path) < 0){
        print("Failed to change directory\n");
        return NULL;
    }
    if(!yaml_has_area(metadata, "ymp")){
        print("Invalid metadata\n");
        return NULL;
    }
    metadata = yaml_get_area(metadata, "ymp");
    if(yaml_has_area(metadata, "source")){
        Archive *a = archive_new();
        archive_load(a, ret);
        archive_set_type(a, "zip", "none");
        char** files = find(path);
        for(size_t i=0; files[i]; i++){
            archive_add(a, files[i]+strlen(path)+1);
        }
        archive_create(a);
        free(a);
        free(files);
    }
    if(chdir(curdir) < 0){
        print("Failed to change directory\n");
        return NULL;
    }
    return ret;
}