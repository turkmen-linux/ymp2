#include <core/ymp.h>
#include <core/variable.h>
#include <core/logger.h>

#include <data/package.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/fetcher.h>
#include <utils/gpg.h>
#include <utils/jobs.h>
#include <utils/archive.h>

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int repo_update_op(const char* uri, const char* repo_name){
    char* metadata = str_replace(uri, "$uri", "ymp-index.yaml");
    char* metadata_gpg = str_replace(uri, "$uri", "ymp-index.yaml.gpg");
    char* name = str_replace(metadata, "/","-");
    char* target = build_string("%s/%s/index/%s.yaml", get_value("DESTDIR"), STORAGE, repo_name);
    char* target_gpg = build_string("%s.gpg", target);
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

static int repo_add(const char* uri){
    int status = 0;
    // fetch repo name
    char* name = get_value("name");
    name = str_replace(name, "/", "-");
    // create sources.list.d if does not exists
    char* sources_path = build_string("%s/%s/sources.list.d/", get_value("DESTDIR"), STORAGE);
    create_dir(sources_path);
    // build file path
    char* repo_path = build_string("%s/%s", sources_path, name);
    FILE* f = fopen(repo_path, "w");
    if(f == NULL){
        status = 1;
        goto repo_add_free;
    }
    fprintf(f, "%s\n", uri);
    // free memory
    fclose(f);
repo_add_free:
    free(repo_path);
    free(sources_path);
    return status;
}

static int repo_index_op(char* file, char** out){
    int status = 1;
    Archive *a = archive_new();
    archive_load(a, file);
    char* metadata = archive_readfile(a, "metadata.yaml");
    if(!metadata){
        status = 1;
        goto repo_index_op_free;
    }
    *out = metadata+5;
repo_index_op_free:
    archive_unref(a);
    return status;
}

static int repo_index(const char* path){
    char* name = get_value("name");
    if(strlen(name) < 1){
        printf("repo name is undefined. Use --name=xxx\n");
        return 1;
    }
    //bool move = iseq(get_value("move"), "true");
    int status = 0;
    char** files = find(path);
    jobs *j = jobs_new();
    // calculate len
    size_t len = 0;
    for(len=0; files[len]; len++){}
    char* out[len];
    // scan all files
    size_t cur = 0;
    for(size_t i=0; files[i]; i++){
        // filter non-ymp files
        if(!endswith(files[i], ".ymp")){
            continue;
        }
        jobs_add(j, (callback)repo_index_op, files[i], &out[cur]);
        cur++;
    }
    jobs_run(j);
    // write index to file
    char index_path[PATH_MAX];
    strcpy(index_path, path);
    strcat(index_path, "/ymp-index.yaml");
    FILE* f = fopen(index_path, "w");
    if(!f){
        perror("Failed to open file:");
        goto repo_index_free;
    }
    fprintf(f, "index:\n");
    fprintf(f, "  name: ");
    fprintf(f, name);
    fprintf(f, "\n");
    for(size_t i=0; i<cur; i++){
        fprintf(f, out[i]);
    }
repo_index_free:
    fclose(f);
    // cleanup memory
    for(size_t i=0; i<cur; i++){
        free(out[i]-5);
    }
    for(size_t i=0; files[i]; i++){
        free(files[i]);
    }
    free(files);
    return status;
}

static int repo_del(){
    int status = 0;
    char* name = get_value("name");
    name = str_replace(name, "/", "-");
    char* repo_path = build_string("%s/%s/sources.list.d/", get_value("DESTDIR"), STORAGE, name);
    if(!isfile(repo_path)){
        status = 1;
        goto repo_del_free;
    }
    unlink(repo_path);
repo_del_free:
    free(repo_path);
    return status;
}

static int repo_main(void** args) {
    if (strcmp(get_value("update"), "true")==0){
        return repo_update(args);
    } else if (strcmp(get_value("add"), "true")==0){
        return repo_add(((char**)args)[0]);
    } else if (strcmp(get_value("remove"), "true")==0){
        return repo_del();
    } else if (strcmp(get_value("index"), "true")==0){
        return repo_index(((char**)args)[0]);
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
