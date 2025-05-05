#include <ymp.h>

#include <stdio.h>
int print(void* args) {
    printf((char*)args);
    return 0;
}

void print_init(OperationManager* manager){
    Operation op;
    op.name = "print";
    op.call = print;
    operation_register(manager, op);
}