#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>

#if __GNU_LIBRARY__
#include <execinfo.h>
#endif 

#include <core/ymp.h>
#include <core/logger.h>

#include <config.h>

#include <utils/process.h>
#include <utils/error.h>
#include <utils/file.h>
#include <utils/string.h>

void ctx_init(OperationManager *manager);

visible ErrorContext exception;

typedef struct {
    const char* name;
    void* args;
} OperationJob;

typedef struct {
    OperationJob *item;
    size_t length;
    size_t capacity;
} YmpPrivate;

Ymp* global;

static YmpPrivate* queue_init(){
    YmpPrivate *queue = (YmpPrivate*) malloc(sizeof(YmpPrivate));
    queue->length = 0;
    queue->capacity = 0;
    queue->item = malloc(sizeof(OperationJob));
    return queue;
}
static void sigsegv_event(int signal){
#if __GNU_LIBRARY__
    void *array[10];
    size_t size;
    char **strings;
    size_t i;
    /* Get backtrace */
    size = backtrace(array, 10);
    strings = backtrace_symbols(array, size);

    printf("Segmentation fault:\n");
    for (i = 0; i < size; i++) {
        printf("%s\n", strings[i]);
    }

    /* Free the memory allocated by backtrace_symbols */
    free(strings);
#endif
    longjmp(exception.buf, signal);
}

visible Ymp* ymp_init(){
    // Allocate memory for Ymp instance
    size_t begin_time = get_epoch();
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
    ymp->priv_data = (void*) queue_init();
    ctx_init(ymp->manager); // Load from ctx
    if(global == NULL) {
        global = ymp;
    }
    if(getenv("DEBUG")){
        logger_set_status(DEBUG, true);
    }
    struct sigaction sigact;
    sigact.sa_handler = sigsegv_event;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGSEGV, &sigact, NULL);
    #ifdef PLUGIN_SUPPORT
    char** plugins = find(PLUGINDIR);
    size_t i = 0;
    while(plugins[i]){
        if(endswith(plugins[i], ".so")){
            load_plugin(ymp, plugins[i]);
            free(plugins[i]);
        }
        i++;
    }
    free(plugins);
    #endif
    debug("ymp init done in %ld µs\n", get_epoch() - begin_time);

    return ymp; // Return the pointer to the newly created instance
}

visible void ymp_unref(Ymp* ymp){
    free(ymp->errors);
    free(ymp->variables);
    free(ymp->manager);
    free(ymp->priv_data);
    free(ymp);
}

visible void ymp_add(Ymp* ymp, const char* name, void* args) {
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

static void ymp_set_logger_status(){
    logger_set_status(DEBUG, get_bool("debug"));
    logger_set_status(INFO, get_bool("verbose"));
}

visible int ymp_run(Ymp* ymp){
    size_t begin_time = get_epoch();
    ymp_set_logger_status();
    YmpPrivate *queue = (YmpPrivate*)ymp->priv_data;
    int rc = 0;
    for(size_t i=0; i< queue->length; i++){
        rc = operation_main(ymp->manager, queue->item[i].name, queue->item[i].args);
        if(rc > 0){
            break;
        }
    }
    free(queue);
    ymp->priv_data = (void*) queue_init();
    debug("ymp run done in %ld µs\n", get_epoch() - begin_time);
    return rc;
}
visible void load_plugin(Ymp* ymp, const char* path){
    (void)ymp; (void) path;
    #ifdef PLUGIN_SUPPORT
    void *handle;
    handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        error_add(build_string("Failed to load plugin: %s from %s\n",dlerror(), path));
        return;
    }
    dlerror();
    void (*plugin_func)(Ymp* ymp);
    *(void**)(&plugin_func) = dlsym(handle, "plugin_init");
    if(!plugin_func){
        return;
    }
    plugin_func(ymp);
    #endif
}

