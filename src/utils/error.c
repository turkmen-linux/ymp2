#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/array.h>
#include <utils/error.h>

visible void error(array* error, int status){
    size_t i;
    size_t len = 0;
    char** errs = array_get(error, &len);
    for(i=0;i<len;i++){
        if(errs[i] != NULL){
            printf("Error: %s\n",errs[i]);
        }
    }
    if(i>0){
        array_unref(error);
        exit(status);
    }
    array_clear(error);
}

visible void error_add(array* error, char* message) {
    if(!error){
        return;
    }
    array_add(error, message);
}

visible bool has_error(array* error){
    if(!error){
        return false;
    }
    return array_length(error) > 0;
}
