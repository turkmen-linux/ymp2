#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>
#include <unistd.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <core/variable.h>

#include <data/repository.h>
#include <data/dependency.h>
#include <data/quarantine.h>

#include <utils/string.h>
#include <utils/array.h>
#include <utils/file.h>
#include <utils/jobs.h>

#include <config.h>

static int remove_package(Package* pi){
    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");

    // build strings
    char* files_path = build_string("%s/%s/files/%s", destdir, STORAGE, pi->name);
    char* links_path = build_string("%s/%s/links/%s", destdir, STORAGE, pi->name);
    char* metadata_path = build_string("%s/%s/metadata/%s.yaml", destdir, STORAGE, pi->name);

    FILE *files = fopen(files_path, "r");
    FILE *links = fopen(files_path, "r");
    array *arr = array_new();

    char line[PATH_MAX + 41]; // line buffer
    char tmp[PATH_MAX + strlen(destdir)]; // temporary buffer
    int status=0;

    // Read each line from the files
    while (fgets(line, sizeof(line), files)) {
        // remove line newline char
        for(size_t i=strlen(line)-1; line[i] == '\n'; i--){
            line[i] = '\0';
        }
        // remove files
        line[40]= '/';
        strcpy(tmp, destdir);
        strcat(tmp,line+40);
        info("Removing: %s\n", tmp);
        if(!isfile(tmp)){
            continue;
        }
        array_add(arr, strdup(tmp));
    }
    size_t offset = 0;
    // Read each line from the links
    while (fgets(line, sizeof(line), links)) {
        // remove line newline char
        for(size_t i=strlen(line)-1; line[i] == '\n'; i--){
            line[i] = '\0';
        }
        // calculate offset
        for(offset=0; line[offset] && line[offset] == ' '; offset++){}
        // remove links
        line[offset]= '/';
        strcpy(tmp, destdir);
        strcat(tmp,line+offset);
        info("Removing: %s\n", tmp);
        if(!issymlink(tmp)){
            continue;
        }
        array_add(arr, strdup(tmp));
    }
    size_t len = 0;
    char** items = array_get(arr, &len);
    for (size_t i=0; i<len;i++){
        if(unlink(items[i]) < 0){
            char* err = build_string("Failed to remove %s", items[i]);
             perror(err);
             free(err);
             status = 1;
             goto free_remove_package;
       }

    }
    // remove files links metadata
    if(unlink(files_path) < 0){
        perror("Failed to remove file list");
    }
    if(unlink(links_path) < 0){
        perror("Failed to remove symlink list");
    }
    if(unlink(metadata_path) < 0){
        perror("Failed to remove metadata");
    }
free_remove_package:
    array_unref(arr);
    free(files_path);
    free(links_path);
    free(metadata_path);
    return status;
}

static int remove_main(void* args){
    char** res = (char**)args;
    Package **pkgs = resolve_reverse_dependency(res[0]);
    jobs *j = jobs_new();
    for(size_t i=0; pkgs[i]; i++){
        jobs_add(j, (callback) remove_package, (void*)pkgs[i], NULL);
    }
    jobs_run(j);
    if(j->failed){
        jobs_unref(j);
        return 1;
    }
    jobs_unref(j);
    return 0;
}

void remove_init(OperationManager* manager){
    Operation op;
    op.name = "remove";
    op.min_args = 1;
    op.call = (callback)remove_main;
    operation_register(manager, op);
}
