#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

#include <core/ymp.h>
#include <core/logger.h>

void ctx_init(OperationManager *manager);

typedef struct {
    const char* name;
    void* args;
} OperationJob;

typedef struct {
    OperationJob *item;
    size_t length;
    size_t capacity;
} YmpPrivate;

visible Ymp* global;

visible Ymp* ymp_init(){
    // Allocate memory for Ymp instance
    Ymp* ymp = (Ymp*)malloc(sizeof(Ymp));
    if(ymp == NULL){
        return NULL; // Memory allocation failed!
    }
    if(!isatty(fileno(stdout))){
        logger_set_status(COLORIZE, false);
    }
    ymp->manager = operation_manager_new(); // Operation manager.
    ymp->variables = variable_manager_new();
    ymp->errors = array_new();
    // Fill private space
    YmpPrivate *queue = (YmpPrivate*) malloc(sizeof(YmpPrivate));
    queue->length = 0;
    queue->capacity = 0;
    queue->item = malloc(sizeof(OperationJob));
    ymp->priv_data = (void*) queue;
    ctx_init(ymp->manager); // Load from ctx
    if(global == NULL) {
        global = ymp;
    }
    return ymp; // Return the pointer to the newly created instance
}

void visible ymp_add(Ymp* ymp, const char* name, void* args) {
    YmpPrivate *queue = (YmpPrivate*)ymp->priv_data;
    if(queue->length >= queue->capacity){
        size_t new_capacity = queue->capacity + 32;
        queue->item = realloc(queue->item, sizeof(OperationJob) * new_capacity);
        if (queue->item == NULL) {
            // Handle memory allocation failure
            fprintf(stderr, "Memory allocation failed\n");
            return;
        }
        queue->capacity = new_capacity;
    }
    queue->item[queue->length].name = name;
    queue->item[queue->length].args = args;
    queue->length++;
}

int visible ymp_run(Ymp* ymp){
    YmpPrivate *queue = (YmpPrivate*)ymp->priv_data;
    global = ymp;
    int rc = 0;
    for(size_t i=0; i< queue->length; i++){
        rc = operation_main(ymp->manager, queue->item[i].name, queue->item[i].args);
        if(rc > 0){
            break;
        }
    }
    global = NULL;
    return rc;
}
visible void load_plugin(const char* path){
    void *handle;
    handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        printf("Failed to load plugin: %s from %s\n ",dlerror(), path);
        return;
    }
    dlerror();
    void (*plugin_func)();
    *(void**)(&plugin_func) = dlsym(handle, "main");
    if(plugin_func){
        plugin_func();
    }
}
