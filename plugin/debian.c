#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <core/operations.h>


#include <utils/archive.h>
#include <utils/file.h>

static VariableManager* vars;

static int deb_extract(char** args){
    char* target = variable_get_value(vars, "target");
    char curdir[PATH_MAX];
    if(!target || strlen(target) == 0){
        if (getcwd(curdir, sizeof(curdir)) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        target = curdir;
    }
    for (size_t i=0; args[i]; i++){
        // 1. extract deb package
        Archive *deb = archive_new();
        archive_load(deb, args[i]);
        archive_set_target(deb, target);
        archive_extract_all(deb);
        archive_unref(deb);
        // 2. extract package files
        char** files = listdir(target);
        char path[PATH_MAX+1];
        char path2[PATH_MAX+8];
        for (size_t j=0; files[j]; j++){
            snprintf(path, sizeof(path), "%s/%s", target, files[j]);
            if(strncmp(files[j], "control.tar.", 12) == 0){
                Archive *control = archive_new();
                archive_load(control, path);
                snprintf(path2, sizeof(path2), "%s/DEBIAN", target);
                archive_set_target(control, path2);
                archive_extract_all(control);
                archive_unref(control);
                unlink(path);
            } else if(strncmp(files[j], "data.tar.", 9) == 0){
                Archive *data = archive_new();
                archive_load(data, path);
                archive_set_target(data, target);
                archive_extract_all(data);
                archive_unref(data);
            }
             unlink(path);
        }
    }
    return 0;
}

static int deb_fn(char** args){
    if (variable_get_value(vars, "extract")){
        return deb_extract(args);
    }
    return 0;
}

visible void plugin_init(Ymp* ymp){
    vars = ymp->variables;
    Operation op;
    op.name = "debian";
    op.description = _("Deb package operations");
    op.alias = "deb";
    op.help = help_new();
    help_add_parameter(op.help, "--extract", _("Extract a deb package."));
    help_add_parameter(op.help, "--target", _("Extract directory target."));
    op.min_args = 0;
    op.call = (callback) deb_fn;
    operation_register(ymp->manager, op);
}
