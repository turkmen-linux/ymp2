#include <stdlib.h>
#include <core/ymp.h>
#include <core/logger.h>

#ifndef LIBYMP

int main(int argc, char** argv){
    Ymp* ymp = ymp_init(); // Create ymp context
    if(argc > 1) {
        ymp_add(ymp, argv[1], argv+2);
    }
    if(getenv("DEBUG") != NULL){
        logger_set_status(DEBUG, true);
    }
    return ymp_run(ymp);
}
#endif
