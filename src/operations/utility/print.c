#include <core/ymp.h>

#include <stdio.h>
static int print(void** args) {
    size_t i=0;
    for(i=0; args[i]; i++){
        printf("%s ", (char*)args[i]);
    }
    if(i>0){
        printf("\n");
    }
    return 0;
}

void print_init(OperationManager* manager){
    Operation op;
    op.name = "print";
    op.alias = "echo";
    op.description = "Print message";
    op.min_args = 0;
    op.help = NULL;
    op.call = (callback)print;
    operation_register(manager, op);
}
