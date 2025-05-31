#include <stdio.h>

#include <core/ymp.h>
#include <core/variable.h>

#include <data/repository.h>

#include <config.h>

#include <utils/yaml.h>
#include <utils/string.h>
#include <utils/file.h>

static void list_available(){
    char* repodir = build_string("%s/%s/index", get_value("DESTDIR"), STORAGE);
    char** dirs = listdir(repodir);
    size_t i=0;
    Repository *repo;
    while(dirs[i]){
        if(!endswith(dirs[i], ".yaml")){
            i++;
            continue;
        }
        repo = repository_new();
        repository_load_from_index(repo, build_string("%s/%s",repodir,dirs[i]));
        for(size_t j=0; j< repo->package_count;j++){
            printf("%s %s\n", repo->packages[j]->name, yaml_get_value(repo->packages[j]->metadata, "description"));
        }
        repository_unref(repo);
        i++;
    }
}

static int list(void** args){
    (void)args;
    if (iseq(get_value("available"), "true")){
        list_available();
    }
    return 0;
}

void list_init(OperationManager* manager){
    Operation op;
    op.name = "list";
    op.call = (callback)list;
    operation_register(manager, op);
}
