#include <string.h>
#include <stdio.h>

#include <core/ymp.h>

#include <data/repository.h>
#include <data/dependency.h>

#include <utils/yaml.h>

static void dump_info(Package *pi){
    if(pi->is_source){
        printf("source:\n");
    } else {
        printf("package:\n");
    }
    printf(
        "name: %s\n"
        "version: %s\n"
        "release %d\n"
        "description; %s\n",
        pi->name,
        pi->version,
        pi->release,
        yaml_get_value(pi->metadata, "description")
    );
    if(package_is_installed(pi)){
        printf("installed: true\n");
    } else {
        printf("installed: false\n");
    }
    printf("dependencies:\n");
    for(size_t i=0; pi->dependencies[i]; i++){
        printf("  - %s\n", pi->dependencies[i]);
    }
    printf("groups:\n");
    for(size_t i=0; pi->groups[i]; i++){
        printf("  - %s\n", pi->groups[i]);
    }

}

static bool print_info(Repository *repo, const char* arg){
    bool ret = false;
    for(size_t j=0; j< repo->package_count;j++){
        Package *pi = repo->packages[j];
        if(pi==NULL){
            continue;
        }
        if(strcmp(pi->name, arg) == 0){
            dump_info(pi);
            ret = true;
        }
    }
    return ret;
}

static int info_main(char** args){
    // init memory
    size_t len = 0;
    for(len=0; args[len]; len++);
    bool is_found[len];
    memset(is_found, false, sizeof(is_found));
    // Begin resolve
    Repository **repos = resolve_begin();
    if(repos == NULL){
        goto info_installed;
    }
    for(size_t i=0; args[i]; i++){
        for(size_t j=0; repos[j]; j++){
            is_found[i] = print_info(repos[j], args[i]);
        }
    }
    // Cleanup memory
    resolve_end(repos);
info_installed:
    // search for installed packages
    for(size_t i=0; args[i]; i++){
        if(is_found[i]){
            continue;
        }
        Package *pi = package_new();
        if(package_load_from_installed(pi, args[i])){
            dump_info(pi);
            package_unref(pi);
        }
    }
    return 0;
}

void info_init(OperationManager* manager){
    Operation op;
    op.name="info";
    op.alias = "i";
    op.description = "Show package information";
    op.min_args = 1;
    op.call = (callback)info_main;
    op.help = NULL;
    operation_register(manager, op);
}
