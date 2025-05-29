#include <stdio.h>
#include <core/ymp.h>

static int set_fn(char** args){
    set_value(args[0], args[1]);
    return 0;
}
static int get_fn(char** args){
    char* value = get_value(args[0]);
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
