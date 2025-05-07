#include <stdio.h>
#include <core/ymp.h>

extern Ymp* global;

int set_fn(char** args){
    variable_set_value(global->variables, args[0], args[1]);
    return 0;
}
int get_fn(char** args){
    char* value = variable_get_value(global->variables, args[0]);
    printf("%s\n", value);
    return 0;
}

void setget_init(OperationManager* manager){
    Operation set;
    set.name = "set";
    set.call = (callback)set_fn;
    operation_register(manager, set);

    Operation get;
    get.name = "get";
    get.call = (callback) get_fn;
    operation_register(manager, get);
}
