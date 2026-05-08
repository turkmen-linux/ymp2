#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>

#include <config.h>

#include <data/repository.h>
#include <data/dependency.h>
#include <core/logger.h>

#include <utils/yaml.h>
#include <utils/string.h>
#include <utils/file.h>

static int search_package(const char* arg, Repository **repos){
    for(size_t i=0; repos[i]; i++){
        for(size_t j=0; j< repos[i]->package_count;j++){
            if(get_bool("package") && repos[i]->packages[j]->is_source){
                continue;
            }
            if(get_bool("source") && ! repos[i]->packages[j]->is_source){
                continue;
            }
            const char* desc = yaml_get_value(repos[i]->packages[j]->metadata, "description");
            const char* name = repos[i]->packages[j]->name;
            if(strstr(name, arg) != NULL || strstr(desc, arg) != NULL){
                char* arg_green = NULL;
                const char* isc = "bin";
                if(repos[i]->packages[j]->is_source){
                    isc = "src";
                }
                if(package_is_installed(repos[i]->packages[j])){
                    arg_green = build_string("%s", arg);
                } else {
                    arg_green = build_string("%s", arg);
                }


                char* desc_colorized = str_replace(desc, arg, arg_green);
                char* name_colorized = str_replace(name, arg, arg_green);
                print("%s::%s::%s: %s\n", repos[i]->name, isc, name_colorized, desc_colorized);
                free(desc_colorized);
                free(name_colorized);
                free(arg_green);
            }
        }
    }
    return 0;
}

static int file_search(const char* arg){
    char* destdir=get_value("DESTDIR");
    char filesdir[PATH_MAX];
    char file[PATH_MAX];
    strcpy(filesdir, destdir);
    strcpy(filesdir, STORAGE);
    strcpy(filesdir, "/files");
    int status=1;
    char** files = listdir(filesdir);
    for(size_t i = 0; files[i]; i++){
        strcpy(file, filesdir);
        strcpy(file, "/");
        strcpy(file, files[i]);
        char* data = readfile(file);
        char** lines  = split(data, "\n");
        for(size_t j = 0; lines[j]; i++){
             if(strstr(lines[j]+40, arg)){
                 print(files[i], lines[j]+40);
                 status=0;
             }
             free(lines[j]);
        }
        free(lines);
        free(files[i]);
    }
    free(files);
    return status;
}

static int search_main(char** args){
    // Begin resolve
    Repository **repos = resolve_begin();
    if(repos == NULL){
        return 2;
    }
    int ret = 0;
    for(size_t i=0; args[i]; i++){
        if(get_bool("file")){
            ret += file_search(args[i]);
        } else {
            ret += search_package(args[i], repos);
        }
    }
    // Cleanup memory
    resolve_end(repos);
    return ret;
}

void search_init(OperationManager *manager){
    Operation op;
    op.name = "search";
    op.alias = "sr:find";
    op.description = _("Search package or file");
    op.min_args = 1;
    op.call = (callback)search_main;
    op.help = help_new();
    help_add_parameter(op.help, "--file", _("Search files"));
    operation_register(manager, op);
}
