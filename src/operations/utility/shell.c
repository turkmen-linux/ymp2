#include <stdio.h>
#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>
#include <core/interpreter.h>

#include <utils/string.h>
#include <utils/process.h>

static int shell_fn(char** args){
    int status = 0;
    for(size_t i=0; args[i]; i++){
        char* data = readfile(args[i]);
        status = run_script(data);
        free(data);
        if(status){
            return status;
        }
    }
    return status;
}

void shell_init(OperationManager* manager){
    Operation op;
    op.name = "shell";
    op.alias = "sh";
    op.help = NULL;
    op.description = _("Execute ympsh file");
    op.min_args = 1;
    op.call = (callback)shell_fn;
    operation_register(manager, op);
}
