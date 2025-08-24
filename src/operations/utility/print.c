#include <core/ymp.h>

#include <stdio.h>
static int print(void** args) {
    for(size_t i=0; args[i]; i++){
        printf("%s ", (char*)args[i]);
    }
    printf("\n");
    return 0;
}

void print_init(OperationManager* manager){
    Operation op;
    op.name = "print";
    op.alias = "echo";
    op.min_args = 0;
    op.call = (callback)print;
    operation_register(manager, op);
}
