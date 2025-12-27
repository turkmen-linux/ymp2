#include <iostream>

extern "C" {

#include <core/ymp.h>
}

int main(int argc, char** args){
    Ymp* ymp = ymp_init();
    char* msg[] = {(char*)"hello world", nullptr};
    ymp_add(ymp, "print", msg);
    int status = ymp_run(ymp);
    return status;
}
