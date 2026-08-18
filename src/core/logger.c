#include <libgen.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/logger.h>
#include <utils/color.h>
#include <utils/process.h>
#include <utils/string.h>

typedef int (*logger)(const char *, ...);

static logger print_functions[] = { (logger) vprintf, NULL, (logger) vprintf, NULL, (logger) vprintf };

visible void logger_set_status(int type, bool status) {
    if (type > (int) (sizeof(print_functions) / sizeof(logger))) {
        return;
    }
    if (status) {
        print_functions[type] = (logger) vprintf;
    } else {
        print_functions[type] = NULL;
    }
}

static size_t cur_time = 0;

visible int print_fn(const char *caller, const char *filename, int line, int type, const char *format, ...) {
    (void) caller;
    if (print_functions[type] == NULL) {
        return 0;
    }

    va_list args;
    va_start(args, format);
    if (type == DEBUG) {
        if (cur_time == 0) {
            cur_time = get_epoch();
        }
        color_print(NORMAL, COLOR_MAGENTA, "[%s:%d (%s) %ld]: ", basename((char *) filename), line, caller, get_epoch() - cur_time);
        cur_time = get_epoch();
    } else if (type == WARNING) {
        color_print(BOLD, COLOR_YELLOW, "%s: ", "Warning");
    } else if (type == INFO) {
        color_print(BOLD, COLOR_CYAN, "%s: ", "INFO");
    } else if (type == ERROR) {
        color_print(BOLD, COLOR_RED, "%s: ", "ERROR");
    }

    int status = print_functions[type](format, args);

    va_end(args);
    return status;
}
