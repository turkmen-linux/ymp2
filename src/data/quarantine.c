#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>


#include <config.h>
#include <core/ymp.h>
#include <core/variable.h>

#include <data/quarantine.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/hash.h>
#include <utils/jobs.h>

static int quarantine_validate_files(const char* name){
    printf("Validate %s (files)\n", name);
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    // validate package files
    int status = 0;
    char* files_path = build_string("%s/%s/quarantine/files/%s", destdir, STORAGE, name);
    if(!isfile(files_path)){
        return 0;
    }
    FILE *files = fopen(files_path, "r");
    char sha1[40];
    char line[1024+41]; // maximum file name length is 1024
    while(fgets(line, sizeof(line), files)){
        // trim line string
        while(line[strlen(line)-1] == '\n'){
            line[strlen(line)-1] = '\0';
        }
        // stop if invalid line detected
        if(strlen(line) <= 40){
            status = 1;
            goto free_quarantine_package;
        }
        // extract sha1 and file path
        strncpy(sha1, line, 40);
        // check file
        char* actual_file = build_string("%s/%s/quarantine/rootfs/%s", destdir, STORAGE, line+41);
        if(!isfile(actual_file)){
            // stop missing file detected
            status = 1;
        } else {
            // calculate hash and check are same
            char* actual_sha1 = calculate_sha1(actual_file);
            status = strncmp(actual_sha1, sha1, 40);
            free(actual_sha1);
        }
        free(actual_file);
        if(status){
            goto free_quarantine_package;
        }
    }
free_quarantine_package:
    // free memory
    free(files_path);
    fclose(files);
    return status;
}

visible bool quarantine_validate(){
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    char* metadata = build_string("%s/%s/quarantine/metadata", destdir, STORAGE);
    char** metadatas = find(metadata);
    jobs* j = jobs_new();
    for(size_t i=0; metadatas[i]; i++){
        if(endswith(metadatas[i], ".yaml")){
            // add jobs
            metadatas[i][strlen(metadatas[i])-5] = '\0';
            jobs_add(j, (callback)quarantine_validate_files, basename(metadatas[i]), NULL);
        }
    }
    // run jobs then unref
    jobs_run(j);
    bool status = j->failed;
    jobs_unref(j);

    // free memory
    for(size_t i=0; metadatas[i]; i++){
        free(metadatas[i]);
    }
    free(metadata);
    free(metadatas);
    return status;
}