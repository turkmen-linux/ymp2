#include <stdio.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <core/variable.h>

#include <data/repository.h>
#include <data/dependency.h>
#include <data/quarantine.h>

#include <config.h>

static int remove_main(void* args){
    char** res = (char**)args;
    for(size_t i=0; res[i];i++){
        print("%s\n", res[i]);
    }
    return 0;
}

void remove_init(OperationManager* manager){
    Operation op;
    op.name = "remove";
    op.call = (callback)remove_main;
    operation_register(manager, op);
}
