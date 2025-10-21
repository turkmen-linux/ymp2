#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/array.h>
#include <utils/error.h>

#include <core/logger.h>
#include <core/ymp.h>

visible void error_fn(array* error, int status){
    if(!error){
        return;
    }
    size_t i;
    size_t len = 0;
    char** errs = array_get(error, &len);
    for(i=0;i<len;i++){
        if(errs[i] != NULL){
            print_fn("",ERROR,errs[i]);
        }
        free(errs[i]);
    }
    free(errs);
    if(i>0){
        array_unref(error);
        exit(status);
    }
    array_clear(error);
}
visible void error(int status){
    error_fn(global->errors, status);
}


visible void error_add_fn(array* error, const char* message) {
    if(!error){
        return;
    }
    array_add(error, message);
}

visible void error_add(const char* message) {
    error_add_fn(global->errors, message);
}

visible bool has_error_fn(array* error){
    if(!error){
        return false;
    }
    return array_length(error) > 0;
}

visible bool has_error(){
    return has_error_fn(global->errors);
}
