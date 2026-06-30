#include <stdio.h>
#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/process.h>

static int exec_fn(char** args){
    char* orig = args[0];
    char* cmd = which(args[0]);
    args[0] = cmd;
    int ret = run_args(args);
    args[0] = orig;
    char* exit_str = int_to_string(ret);
    set_value("EXIT_CODE", exit_str);
    free(exit_str);
    free(cmd);
    return ret;
}

void exec_init(OperationManager* manager){
    Operation op;
    op.name = "exec";
    op.alias = "exec:run";
    op.help = NULL;
    op.description = _("Execute command");
    op.min_args = 1;
    op.call = (callback)exec_fn;
    operation_register(manager, op);
}
