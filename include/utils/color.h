/**
 * @file color.h
 * @brief Terminal color utilities for styled console output.
 */

#ifndef COLOR_H
#define COLOR_H

#include <stdio.h>
#include <stdarg.h>

/**
 * @brief ANSI terminal color codes.
 */
typedef enum {
    COLOR_DEFAULT = -1,   ///< Default terminal color
    COLOR_BLACK   = 30,   ///< Black
    COLOR_RED     = 31,   ///< Red
    COLOR_GREEN   = 32,   ///< Green
    COLOR_YELLOW  = 33,   ///< Yellow
    COLOR_BLUE    = 34,   ///< Blue
    COLOR_MAGENTA = 35,   ///< Magenta
    COLOR_CYAN    = 36,   ///< Cyan
    COLOR_LIGHT_GRAY = 37,///< Light gray
    COLOR_DARK_GRAY  = 90,///< Dark gray
    COLOR_LIGHT_RED  = 91,///< Light red
    COLOR_LIGHT_GREEN= 92,///< Light green
    COLOR_LIGHT_YELLOW=93,///< Light yellow
    COLOR_LIGHT_BLUE =94, ///< Light blue
    COLOR_LIGHT_MAGENTA=95,///< Light magenta
    COLOR_LIGHT_CYAN =96, ///< Light cyan
    COLOR_WHITE      = 97 ///< White
} color_t;

#define BOLD true
#define NORMAL false

/**
 * @brief Print formatted text with ANSI color and optional bold styling.
 *
 * @param bold  If true, prints text in bold
 * @param color ANSI color code from color_t
 * @param fmt   Printf-style format string
 * @param ...   Format arguments
 */
void color_print(bool bold, color_t color, const char *fmt, ...);

#endif
