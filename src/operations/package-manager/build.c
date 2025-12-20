#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>
#include <data/build.h>
#include <utils/file.h>
#include <utils/string.h>

#include <stdio.h>
#include <string.h>

static int build(void** args) {
    char* cache;
    for(size_t i=0; args[i]; i++){
        cache = build_source_from_path(args[i]);
        debug("Bulid cache %s\n",cache);
        if(!isdir(cache)){
            return 1;
        }
        char* build = build_binary_from_path(cache);
        if(build == NULL){
            free(cache);
            return 1;
        }
        char* pkg = create_package(cache);
        debug("Output package %s %s %d\n",pkg, args[i], i);

        char* fname = ympbuild_package_filename(args[i]);
        // move binary package file to output
        char* target = strdup(get_value("output"));
        if(strlen(target) == 0){
            target = strdup(args[i]);
        }
        char* target_file = build_string("%s/%s", target, fname);
        create_dir(target);
        (void)move_file(pkg, target_file);
        // free memory
        free(pkg);
        free(target);
        free(target_file);
        free(fname);
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

