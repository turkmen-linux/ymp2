#include <stdio.h>

#include <core/ymp.h>
#include <core/variable.h>

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
            printf("%s %s\n", name, desc);
        }
        i++;
    }
    resolve_end(repos);
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
    op.alias = "ls";
    op.min_args = 0;
    op.call = (callback)list;
    operation_register(manager, op);
}
