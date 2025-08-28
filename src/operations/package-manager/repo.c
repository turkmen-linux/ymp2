#include <core/ymp.h>
#include <core/variable.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/fetcher.h>
#include <utils/gpg.h>

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int repo_update_op(const char* uri, const char* repo_name){
    char* metadata = str_replace(uri, "$uri", "ymp-index.yaml");
    char* metadata_gpg = str_replace(uri, "$uri", "ymp-index.yaml.gpg");
    char* name = str_replace(metadata, "/","-");
    char* target = build_string("%s/%s/index/%s", get_value("DESTDIR"), STORAGE, name);
    char* target_gpg = build_string("%s/%s/index/%s.gpg", get_value("DESTDIR"), STORAGE, name);
    char* keyring = build_string("%s/%s/gpg/%s.gpg", get_value("DESTDIR"), STORAGE, repo_name);
    debug("update: %s => %s\n", name, target);
    int status = 0;
    if(!fetch(metadata, target)){
        status = 1;
        goto repo_update_op_free;
    }
    if(!fetch(metadata_gpg, target_gpg)){
        status = 1;
        goto repo_update_op_free;
    }
    if(!verify_file(target, keyring)){
        status = 1;
        goto repo_update_op_free;
    }
repo_update_op_free:
    // free memory
    free(target);
    free(target_gpg);
    free(name);
    free(metadata);
    free(metadata_gpg);
    free(keyring);
    return status;
}

static int repo_update() {
    char* repo_path = build_string("%s/%s/sources.list.d", get_value("DESTDIR"), STORAGE);
    char** repos = listdir(repo_path);
    int status = 0;
    for(size_t r=0; repos[r]; r++){
        char* repo_file_path = build_string("%s/%s/sources.list.d/%s", get_value("DESTDIR"), STORAGE, repos[r]);
        if(repo_file_path[0] == '.' || !isfile(repo_file_path)){
            free(repos[r]);
            free(repo_file_path);
            continue;
        }
        char* repo_ctx = trim(readfile(repo_file_path));
        char** repo_urls = split(repo_ctx, "\n");
        for(size_t i=0; repo_urls[i]; i++){
            status += repo_update_op(repo_urls[i], repos[r]);
            free(repo_urls[i]);
        }
        // free memory
        free(repo_file_path);
        free(repo_ctx);
        free(repos[r]);
    }
    // free memory
    free(repo_path);
    free(repos);
    return status;
}

static int repo_main(void** args) {
    if (strcmp(get_value("update"), "true")==0){
        return repo_update(args);
    }
    return 0;
}

void repo_init(OperationManager* manager){
    Operation op;
    op.name = "repo";
    op.alias = "";
    op.description = "Repository operations";
    op.min_args = 0;
    op.call = (callback)repo_main;
    operation_register(manager, op);
}
