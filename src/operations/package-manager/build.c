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
        (void)pkg;
        debug("Output package %s\n",pkg);
        char* build = build_binary_from_path(cache);
        if(build){
            create_package(build);
            free(build);
        } else {
            free(cache);
            return 1;
        }
        free(cache);
    }
    return 0;
}

void build_init(OperationManager* manager){
    Operation op;
    op.name = "build";
    op.description = "Build package";
    op.help = NULL;
    op.alias = NULL;
    op.call = (callback)build;
    op.min_args = 1;
    operation_register(manager, op);
}

