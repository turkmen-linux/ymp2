#include <core/ymp.h>

extern Ymp* global;

#include <stdio.h>
int print(void* args) {
    printf("%s\n", (char*)args);
    return 0;
}

void print_init(OperationManager* manager){
    Operation op;
    op.name = "print";
    op.call = (callback)print;
    operation_register(manager, op);
}
