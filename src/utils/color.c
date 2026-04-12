#include <utils/color.h>

visible void color_print(bool bold, int color, const char *fmt, ...) {
    va_list ap;

    if (bold) {
        printf("\x1b[1;%dm", color+30);
    }  else {
        printf("\x1b[%dm", color+30);
    }

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    printf("\x1b[0m");
}
