#include <stdio.h>
#include <stdlib.h>

#include <core/operations.h>


int my_print(void* args){
    printf("%s\n", (char*)args);
    return 0;
}

int gen_err(void* args){
    (void) args;
    return 1; // >0 is error
}

int err_handler(void* args) {
    (void) args;
    printf("Error detected!\n");
    return 0;
}

int main(int argc, char** argv){
    (void) argc; (void) argv;
    OperationManager *manager = operation_manager_new();

    // operation 1
    Operation op;
    op.name = "foo";
    op.alias = NULL;
    op.min_args = 1;
    op.call = (callback)my_print;
    operation_register(manager, op);

    // operation 2 (will generate an error)
    Operation op2;
    op2.name = "bar";
    op2.alias = NULL;
    op2.min_args = 1;
    op2.call = (callback)gen_err;
    operation_register(manager, op2);

    // error handler
    Operation op3;
    op3.alias = NULL;
    op3.min_args = 0;
    op3.call = (callback)err_handler;
    manager->on_error = op3;

    int rc = 0;
    rc += operation_main(manager, "foo", "hello foo\n");
    rc += operation_main(manager, "bar", "hello bar\n");
    free(manager);

    if(rc > 0){
        return 0;
    };

}
