#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/file.h>

#include <config.h>

#ifndef LIBYMP

int main(int argc, char** argv){
    Ymp* ymp = ymp_init(); // Create ymp context
    char** plugins = find(PLUGINDIR);
    size_t i = 0;
    while(plugins[i]){
        if(endswith(plugins[i], ".so")){
            load_plugin(ymp, plugins[i]);
        }
        i++;
    }
    if(argc > 1) {
        ymp_add(ymp, argv[1], argv+2);
    }
    if(getenv("DEBUG") != NULL){
        logger_set_status(DEBUG, true);
    }
    return ymp_run(ymp);
}
#endif
