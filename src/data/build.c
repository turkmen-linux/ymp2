#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>

#include <core/ymp.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>
#include <utils/fetcher.h>
#include <utils/archive.h>

#include <data/build.h>
#include <config.h>

visible char* ympbuild_get_value(ympbuild* ymp, const char* name) {
    char* command = build_string(
    "exec <&-\n"
    "{\n%s\n} &>/dev/null\n"
    "echo -n ${%s}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput(args));
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
    char* output = strip(getoutput(args));
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

static void configure_header(ympbuild *ymp) {
    if(!global){
        return;
    }
    ymp->header = str_replace(ymp->header, "@buildpath@", ymp->path);
    ymp->header = str_replace(ymp->header, "@CC@", variable_get_value(global->variables, "build:cc"));
    ymp->header = str_replace(ymp->header, "@CXX@", variable_get_value(global->variables, "build:cxx"));
    ymp->header = str_replace(ymp->header, "@CFLAGS@", variable_get_value(global->variables, "build:cflags"));
    ymp->header = str_replace(ymp->header, "@CXXFLAGS@", variable_get_value(global->variables, "build:cxxflags"));
    ymp->header = str_replace(ymp->header, "@LDFLAGS@", variable_get_value(global->variables, "build:ldflags"));
    ymp->header = str_replace(ymp->header, "@APIKEY@", variable_get_value(global->variables, "build:token"));
    ymp->header = str_replace(ymp->header, "@DISTRODIR@", DISTRODIR);
}

static void generate_links_files(const char* path){
    char* rootfs = build_string("%s/output", path);
    char** inodes = find(rootfs);
    array *files = array_new();
    array *links = array_new();
    for(size_t i=0; inodes[i]; i++){
        if(issymlink(inodes[i])){
            printf("%s\n", inodes[i]);
            array_add(links, build_string("%s %s\n",sreadlink(inodes[i]), inodes[i]+strlen(rootfs)+1));
        } else if(isfile(inodes[i])){
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

visible bool build_from_path(const char* path){
    char* ympfile = build_string("%s/ympbuild",path);
    if(!isfile(ympfile)){
        free(ympfile);
        return false;
    }
    // configure header
    ympbuild *ymp = malloc(sizeof(ympbuild));
    ymp->ctx = readfile(ympfile);
    ymp->header = readfile(":/ympbuild-header.sh");
    // Create build path
    ymp->path = calculate_md5(ympfile);
    ymp->path = build_string("%s/%s", BUILD_DIR, ymp->path);
    configure_header(ymp);
    create_dir(ymp->path);
    // fetch values
    char* name = ympbuild_get_value(ymp, "name");
    char** deps = ympbuild_get_array(ymp, "depends");
    char** sources = ympbuild_get_array(ymp, "source");
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
    char* target = build_string("%s/cache/%s/ympbuild",BUILD_DIR, name);
    copy_file(ympfile, target);
    free(target);
    // copy resources
    for(size_t i=0; sources[i] && hashs[i]; i++){
        if(!get_resource(path, name, hash_type, sources[i], hashs[i])){
            return false;
        }
    }
    char* src_cache = build_string("%s/cache/%s/",BUILD_DIR, name);
    char** src_files = find(src_cache);
    Archive *a = archive_new();
    for(size_t i=0; src_files[i];i++){
        if(archive_is_archive(a, src_files[i])){
            archive_load(a, src_files[i]);
            archive_set_target(a, ymp->path);
            archive_extract_all(a);
        } else {
            char* src_name = src_files[i]+strlen(src_cache);
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
            return false;
        }
    }
    // Generate links file
    generate_links_files(ymp->path);
    (void)name; (void)deps;
    // Cleanup
    free(ympfile);
    return true;
}
