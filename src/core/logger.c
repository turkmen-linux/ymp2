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

static const char* colorize_func(const char* colorized, const char* flat){
    (void)flat;
    return colorized;
}

static const char* colorize_dummy(const char* colorized, const char* flat){
    (void)colorized;
    return flat;
}

visible Colorize colorize_fn = (Colorize)colorize_func;


visible void logger_set_status(int type, bool status){
    if(type == COLORIZE){
        if(status){
            colorize_fn = (Colorize)colorize_func;
        } else {
            colorize_fn = (Colorize)colorize_dummy;
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
        printf(colorize_fn("["colorized(BLUE,"%s")":%ld]: ", "[%s:%ld]: "), caller, get_epoch() - cur_time);
        cur_time = get_epoch();
    }else if(type == ERROR){
        printf("%s: ", colorize(RED, "ERROR"));
    }

    int status = print_functions[type](format, args);

    va_end(args);
    return status;
}
