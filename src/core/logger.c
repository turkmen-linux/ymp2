#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <utils/string.h>
#include <utils/process.h>

#include <core/logger.h>

typedef int (*logger)(const char*, ...);


static logger print_functions[] = {(logger)vprintf, NULL, (logger)vprintf, NULL, (logger)vprintf};

static char* colorize_fn(int color, const char* message){
    return build_string("\x1b[;%dm%s\x1b[;0m", color, message);
}

static char* colorize_dummy(int color, const char* message){
    (void)color;
    return strdup(message);
}

visible Colorize colorize = (Colorize)colorize_fn;


visible void logger_set_status(int type, bool status){
    if(type == COLORIZE){
        if(status){
            colorize = (Colorize)colorize_fn;
        } else {
            colorize = (Colorize)colorize_dummy;
        }
    }
    if(type > (int)(sizeof(print_functions) / sizeof(logger))){
        return;
    }
    if(status){
        print_functions[type] = (logger)vprintf;
    } else {
        print_functions[type] = NULL;
    }
}

static size_t cur_time = 0;

visible int print_fn(const char* caller, int type, const char* format, ...){
    (void)caller;
    if(print_functions[type] == NULL){
        return 0;
    }

    va_list args;
    va_start(args, format);
    if(type == DEBUG){
        if(cur_time == 0){
            cur_time = get_epoch();
        }
        char* msg = colorize(BLUE, (char*)caller);
        printf("[%s:%ld]: ", msg, get_epoch() - cur_time);
        cur_time = get_epoch();
        free(msg);
    }else if(type == ERROR){
        char* msg = colorize(RED, "ERROR");
        printf("%s: ", msg);
        free(msg);
    }

    int status = print_functions[type](format, args);

    va_end(args);
    return status;
}
