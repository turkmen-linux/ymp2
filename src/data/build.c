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
        execvp(args[0], args);
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

static bool get_resource(const char* path, const char* name, size_t type, const char* source, const char* hash){
    debug("Source: %s %s\n", source, hash);
    // Get file name
    char* source_name = basename((char*)source);
    // Get target path
    char* target = build_string("%s/cache/%s",BUILD_DIR, name);
    char* source_target = build_string("%s/%s",target, source_name);
    bool status = true;
    if(!isfile(source_target)){
        // Download or Copy
        create_dir(target);
        char* local_file = build_string("%s/%s", path, source);
        if(isfile(local_file)){
            free(source_target);
            source_target = build_string("%s/%s",target, source);
            status = copy_file(local_file, source_target);
        } else {
            status = fetch(source, source_target);
        }
        free(local_file);
    }
    // Check archive hash
    char* data_hash = calculate_hash(type, source_target);
    if(iseq((char*)hash, "SKIP")){
        warning("Hash control disabled for: %s\n", source_name);
    }else if(!iseq(data_hash, (char*)hash)){
        print("Archive hash is invalid:\n  -> Excepted: %s\n  -> Received: %s\n", hash, data_hash);
        return false;
    }
    // Cleanup
    free(target);
    free(source_target);
    return status;
}

static char* actions[] = {"prepare", "setup", "build", "package", NULL};

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
    ymp->header = str_replace(ymp->header, "@buildpath@", ymp->path);
    ymp->header = str_replace(ymp->header, "@CC@", "gcc");
    ymp->header = str_replace(ymp->header, "@CXX@", "g++");
    ymp->header = str_replace(ymp->header, "@CFLAGS@", "");
    ymp->header = str_replace(ymp->header, "@CXXFLAGS@", "");
    ymp->header = str_replace(ymp->header, "@LDFLAGS@", "");
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
    (void)name; (void)deps;
    // Cleanup
    free(ympfile);
    return true;
}
