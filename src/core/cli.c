#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>
#include <core/interpreter.h>

#include <utils/string.h>
#include <utils/file.h>

#include <config.h>

int ymp_main(int argc, char** argv){
    Ymp* ymp = ymp_init(); // Create ymp context
    #ifdef PLUGIN_SUPPORT
    char** plugins = find(PLUGINDIR);
    size_t i = 0;
    while(plugins[i]){
        if(endswith(plugins[i], ".so")){
            load_plugin(ymp, plugins[i]);
        }
        i++;
    }
    #endif
    if(argc > 1) {
        if(isfile(argv[1])){
            return run_script(readfile(argv[1]));
        }
        ymp_add(ymp, argv[1], argv+2);
    }
    if(getenv("DEBUG") != NULL){
        logger_set_status(DEBUG, true);
    }
    return ymp_run(ymp);
}
