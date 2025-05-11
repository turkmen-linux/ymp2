#include <core/ymp.h>

#ifndef LIBYMP

int main(int argc, char** argv){
    Ymp* ymp = ymp_init(); // Create ymp context
    if(argc > 1) {
        ymp_add(ymp, argv[1], argv+2);
    }
    return ymp_run(ymp);
}
#endif
