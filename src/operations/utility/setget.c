#include <stdio.h>
#include <core/ymp.h>

#include <utils/string.h>

static int set_fn(char** args){
    set_value(args[0], args[1]);
    return 0;
}
static int get_fn(char** args){
    char* value = get_value(args[0]);
    printf("%s\n", value);
    return 0;
}

static int eq_fn(char** args){
    if(iseq(args[0], args[1])){
        return 0;
    } else {
        return 1;
    }
}

static int dummy_fn(char** args){
    (void)args;
    return 0;
}
void setget_init(OperationManager* manager){
    Operation set;
    set.name = "set";
    set.min_args = 2;
    set.call = (callback)set_fn;
    operation_register(manager, set);

    Operation get;
    get.name = "get";
    get.min_args = 1;
    get.call = (callback) get_fn;
    operation_register(manager, get);

    Operation eq;
    eq.name = "eq";
    eq.min_args = 2;
    eq.call = (callback) eq_fn;
    operation_register(manager, eq);

    Operation dummy;
    dummy.name = ":";
    dummy.min_args = 0;
    dummy.call = (callback) dummy_fn;
    operation_register(manager, dummy);
}
