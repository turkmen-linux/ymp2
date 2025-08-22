#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <core/logger.h>
#include <core/operations.h>

typedef struct {
    bool running;
} OperationManagerPriv;

visible OperationManager* operation_manager_new() {
    // Allocate memory for the OperationManager instance
    OperationManager *manager = (OperationManager *)malloc(sizeof(OperationManager));
    manager->priv_data = (void*)malloc(sizeof(OperationManagerPriv));
    if (manager == NULL) {
        return NULL; // Memory allocation failed
    }

    // Initialize the members of the OperationManager
    manager->operations = NULL; // Initially set to NULL
    manager->length = 0;        // Length is set to zero
    manager->capacity = 0;      // Capacity is set to zero

    // Private area
    OperationManagerPriv* priv = (OperationManagerPriv*)manager->priv_data;
    priv->running = false;

    return manager; // Return the pointer to the newly created instance
}


void visible operation_register(OperationManager *manager, Operation new_op) {
    // Check if we need to resize the array
    if (manager->length >= manager->capacity) {
        size_t new_capacity = manager->capacity + 32; // Increase capacity by 32
        Operation *new_ops = realloc(manager->operations, sizeof(Operation) * new_capacity);

        // Check if realloc was successful
        if (new_ops == NULL) {
            // Handle memory allocation failure (e.g., log an error, exit, etc.)
            fprintf(stderr, "Memory allocation failed\n");
            return;
        }

        // Update the manager's ops and capacity
        manager->operations = new_ops;
        manager->capacity = new_capacity;
    }

    // Add the new operation to the array
    manager->operations[manager->length] = new_op; // Assuming Operation has a proper assignment operator
    manager->length++; // Increment the count of operations
}

int visible operation_main(OperationManager *manager, const char* name, void* args){
    if(name && strlen(name) <= 0){
        return 0;
    }
    // calculate arg len
    size_t len = 0;
    for(len=0; ((char**)args)[len]; len++){}
    int status = 0;
    OperationManagerPriv* priv = (OperationManagerPriv*)manager->priv_data;
    for (size_t i = 0; i < manager->length; i++) {
        if (strcmp(manager->operations[i].name, name) == 0) {
            if(len < manager->operations[i].min_args){
                debug("Min arguments error\n");
                status = 1;
                goto operation_main_on_error;
            }
            debug("Running:%s\n", name);
            priv->running = true;
            status = manager->operations[i].call(args);
            priv->running = false;
            if(status > 0){
operation_main_on_error:
                if (manager->on_error.call){
                    manager->on_error.call(NULL);
                }
                break;
            }
        }
    }
    debug("exit with:%d\n", status);
    return status;
}
