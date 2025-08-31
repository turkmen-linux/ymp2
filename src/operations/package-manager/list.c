#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <core/variable.h>
#include <core/logger.h>

#include <data/repository.h>
#include <data/dependency.h>

#include <config.h>

#include <utils/yaml.h>
#include <utils/string.h>
#include <utils/file.h>

static void list_available(){
    size_t i=0;
    Repository **repos = resolve_begin();
    while(repos[i]){
        for(size_t j=0; j< repos[i]->package_count;j++){
            const char* name = repos[i]->packages[j]->name;
            const char* desc = yaml_get_value(repos[i]->packages[j]->metadata, "description");
            char* meta = build_string("%s/%s/metadata/%s.yaml", get_value("DESTDIR"), STORAGE, name);
            if(isfile(meta)){
                printf("%s %s\n", colorize(GREEN,name), desc);
            } else {
                printf("%s %s\n", colorize(RED,name), desc);
            }
            free(meta);
        }
        i++;
    }
    resolve_end(repos);
}

static void list_installed(){
    char* destdir = get_value("DESTDIR");
    char* metadata = build_string("%s/%s/metadata/", destdir, STORAGE);
    char** meta = listdir(metadata);
    for(size_t i=0; meta[i]; i++){
        if(!endswith(meta[i], ".yaml")){
            continue;
        }
        meta[i][strlen(meta[i])-5] = '\0';
        Package *pi = package_new();
        bool load = package_load_from_installed(pi, meta[i]);
        if(load){
            const char* desc = yaml_get_value(pi->metadata, "description");
            printf("%s %s\n", colorize(GREEN,pi->name), desc);
        } else {
            warning("Failed to read package metadata: %s\n", meta[i]);
        }
        // free memory
        package_unref(pi);
        free(meta[i]);
    }
    free(metadata);
    free(meta);
}

static int list(void** args){
    (void)args;
    if (iseq(get_value("available"), "true")){
        list_available();
    } else if (iseq(get_value("installed"), "true")){
        list_installed();
    }
    return 0;
}

void list_init(OperationManager* manager){
    Operation op;
    op.name = "list";
    op.alias = "ls";
    op.description = "List packages, repos and more";
    op.min_args = 0;
    op.call = (callback)list;
    operation_register(manager, op);
}
