#include <string.h>
#include <stdio.h>

#include <core/ymp.h>

#include <data/repository.h>
#include <data/dependency.h>

#include <utils/yaml.h>
#include <utils/color.h>

static void dump_info(Package *pi){
    if(pi->is_source){
        color_print(BOLD, COLOR_CYAN, "source:\n");
    } else {
        color_print(BOLD, COLOR_CYAN, "package:\n");
    }
    const char* desc = yaml_get_value(pi->metadata, "description");
    if(desc == NULL){
        return;
    }
    color_print(BOLD, COLOR_YELLOW, "  name: ");
    color_print(NORMAL, COLOR_DEFAULT, "%s\n", pi->name);
    color_print(BOLD, COLOR_YELLOW, "  version: ");
    color_print(NORMAL, COLOR_DEFAULT, "%s\n", pi->version);
    color_print(BOLD, COLOR_YELLOW, "  release: ");
    color_print(NORMAL, COLOR_DEFAULT, "%d\n", pi->release);
    color_print(BOLD, COLOR_YELLOW, "  description: ");
    color_print(NORMAL, COLOR_DEFAULT, "%s\n", desc);
    if(package_is_installed(pi)){
        color_print(BOLD, COLOR_GREEN, "  installed: true\n");
    } else {
        color_print(BOLD, COLOR_RED, "  installed: false\n");
    }
    color_print(BOLD, COLOR_YELLOW, "  dependencies:\n");
    for(size_t i=0; pi->dependencies[i]; i++){
        color_print(NORMAL, COLOR_CYAN, "    - %s\n", pi->dependencies[i]);
    }
    color_print(BOLD, COLOR_YELLOW, "  groups:\n");
    for(size_t i=0; pi->groups[i]; i++){
        color_print(NORMAL, COLOR_MAGENTA, "    - %s\n", pi->groups[i]);
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
    op.description = _("Show package information");
    op.min_args = 1;
    op.call = (callback)info_main;
    op.help = NULL;
    operation_register(manager, op);
}
