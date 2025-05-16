#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <utils/string.h>
#include <core/logger.h>

typedef int (*logger)(const char*, ...);

static logger print_functions[] = {(logger)vprintf, NULL, (logger)vprintf, NULL};

visible void logger_set_status(int type, bool status){
    if(type > (int)(sizeof(print_functions) / sizeof(logger))){
        return;
    }
    if(status){
        print_functions[type] = (logger)vprintf;
    } else {
        print_functions[type] = NULL;
    }
}

visible char* colorize(int color, const char* message){
    if(!isatty(fileno(stdout))){
        return strdup(message);
    }
    return build_string("\x1b[;%dm%s\x1b[;0m", color, message);
}

visible int print_fn(const char* caller, int type, const char* format, ...){
    if(print_functions[type] == NULL){
        return 0;
    }

    va_list args;
    va_start(args, format);
    if(type == DEBUG){
        char* msg = colorize(BLUE,caller);
        printf("[%s]:", msg);
        free(msg);
    }

    int status = print_functions[type](format, args);

    va_end(args);
    return status;
}
