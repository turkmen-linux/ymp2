#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <utils/color.h>
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
    if(repos == NULL){
        return;
    }
    while(repos[i]){
        for(size_t j=0; j< repos[i]->package_count;j++){
            const char* name = repos[i]->packages[j]->name;
            const char* desc = yaml_get_value(repos[i]->packages[j]->metadata, "description");
            if(name == NULL || desc == NULL){
                continue;
            }
            char* meta = build_string("%s/%s/metadata/%s.yaml", get_value("DESTDIR"), STORAGE, name);
            if(isfile(meta)){
                color_print(BOLD, COLOR_GREEN, "i %s ::", name);
                color_print(NORMAL, COLOR_WHITE, " %s\n", desc);
            } else {
                color_print(BOLD, COLOR_YELLOW, "r %s ::", name);
                color_print(NORMAL, COLOR_WHITE, " %s\n", desc);
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
            color_print(BOLD, COLOR_GREEN, "i %s ::", pi->name);
            color_print(NORMAL, COLOR_WHITE, " %s\n", desc);
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
    if (get_bool("available")){
        list_available();
    } else if (get_bool("installed")){
        list_installed();
    }
    return 0;
}

void list_init(OperationManager* manager){
    Operation op;
    op.name = "list";
    op.alias = "ls";
    op.description = _("List packages, repos and more");
    op.min_args = 0;
    op.help = help_new();
    help_add_parameter(op.help, "--available", _("List available packages"));
    help_add_parameter(op.help, "--installed", _("List installed packages"));
    op.call = (callback)list;
    operation_register(manager, op);
}
