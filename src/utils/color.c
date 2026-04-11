#include <utils/color.h>

visible void color_print(bool bold, color_t color, const char *fmt, ...) {
    va_list ap;

    if (color != COLOR_DEFAULT) {
        if (bold) {
            printf("\x1b[1;%dm", color);
        }
        else {
            printf("\x1b[%dm", color);
        }
    } else if (bold) {
        printf("\x1b[1m");
    }

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    if (color != COLOR_DEFAULT || bold) printf("\x1b[0m");
}
