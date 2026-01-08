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
        char* target = get_value("output");

        if(strlen(target) == 0){
            target = args[i];
        }
        cache = build_source_from_path(args[i]);
        // create source package
        char* spkg = create_package(cache);
        char* sname = ympbuild_source_filename(args[i]);
        char* target_sfile = build_string("%s/%s", target, sname);
        (void)move_file(spkg, target_sfile);

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

        char* pname = ympbuild_package_filename(args[i]);
        // move binary package file to output
        char* target_pfile = build_string("%s/%s", target, pname);
        create_dir(target);
        (void)move_file(pkg, target_pfile);
        // free memory
        free(pkg);
        free(pname);
        free(sname);
        free(cache);
        // install package
        if(get_bool("install")){
            char* iargs[] = {target_pfile, NULL};
            int status = operation_main(global->manager, "install", iargs);
            if(status != 0){
                free(target_pfile);
                free(target_sfile);
                return status;
            }
        }
        free(target_pfile);
        free(target_sfile);
    }
    return 0;
}

void build_init(OperationManager* manager){
    Operation op;
    op.name = "build";
    op.description = "Build package";
    op.alias = "bi:make";
    op.help = help_new();
    help_add_parameter(op.help, "--install", "install after build");
    op.call = (callback)build;
    op.min_args = 1;
    operation_register(manager, op);
}

