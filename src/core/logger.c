#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include <core/logger.h>

typedef int (*logger)(const char*, ...);

static int dummy(const char* format, ...){
    (void)format;
    return 0;
}

static logger print_functions[] = {(logger)vprintf, (logger)dummy, (logger)dummy, (logger)dummy};

visible void logger_set_status(int type, bool status){
    if(type > (int)(sizeof(print_functions) / sizeof(logger))){
        return;
    }
    if(status){
        print_functions[type] = (logger)vprintf;
    } else {
        print_functions[type] = dummy;
    }
}

visible int print_fn(int type, const char* format, ...){
    va_list args;
    va_start(args, format);

    int status = print_functions[type](format, args);

    va_end(args);
    return status;
}
