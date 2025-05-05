#include <stdlib.h>

#include <ymp.h>

void ctx_init(OperationManager *manager);

visible Ymp* ymp_init(){
    // Allocate memory for Ymp instance
    Ymp* ymp = (Ymp*)malloc(sizeof(Ymp));
    if(ymp == NULL){
        return NULL; // Memory allocation failed!
    }
    ymp->manager = operation_manager_new(); // Operation manager.
    ctx_init(ymp->manager); // Load from ctx
    return ymp; // Return the pointer to the newly created instance
}