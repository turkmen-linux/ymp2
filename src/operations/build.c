#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>
#include <data/build.h>
#include <utils/file.h>

#include <stdio.h>
static int build(void** args) {
    char* cache;
    for(size_t i=0; args[i]; i++){
        cache = build_source_from_path(args[i]);
        debug("Bulid cache %s\n",cache);
        if(!isdir(cache)){
            return 1;
        }
        char* pkg = create_package(cache);
        debug("Output package %s\n",pkg);
        free(cache);
    }
    return 0;
}

void build_init(OperationManager* manager){
    Operation op;
    op.name = "build";
    op.call = (callback)build;
    operation_register(manager, op);
}

