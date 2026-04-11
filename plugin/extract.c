#include <string.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <utils/archive.h>

static VariableManager* vars;

static int extract_main(char**args){
    for(size_t i=0; args[i]; i++){
        Archive *arr = archive_new();
        archive_load(arr, args[i]);
        if(strcmp(variable_get_value(vars, "target"), "") == 0){
            archive_set_target(arr, "./");
        } else {
            archive_set_target(arr, variable_get_value(vars, "target"));
        }
        archive_extract_all(arr);
        archive_unref(arr);
    }
    return 0;
}

visible void plugin_init(Ymp* ymp){
    vars = ymp->variables;
    Operation op;
    op.name = "extract";
    op.description = _("Extract files from archive");
    op.alias = "x";
    op.help = help_new();
    help_add_parameter(op.help, "--target", _("Target directory"));
    op.min_args = 1;
    op.call = (callback) extract_main;
    operation_register(ymp->manager, op);
}