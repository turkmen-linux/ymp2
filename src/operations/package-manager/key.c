#include <core/ymp.h>

static int key_main(char** args){
    (void)args;
    return 0;
}

void key_init(OperationManager *manager){
    Operation op;
    op.name = "key";
    op.alias = "k";
    op.description = "Ymp key manager";
    op.min_args = 0;
    op.call = (callback)key_main;
    operation_register(manager, op);
}