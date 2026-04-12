/**
 * @file color.h
 * @brief Terminal color utilities for styled console output.
 */

#ifndef COLOR_H
#define COLOR_H

#include <stdio.h>
#include <stdarg.h>

#include <ncurses.h>

#define BOLD true
#define NORMAL false

/**
 * @brief Print formatted text with ANSI color and optional bold styling.
 *
 * @param bold  If true, prints text in bold
 * @param color ANSI color code from ncurses
 * @param fmt   Printf-style format string
 * @param ...   Format arguments
 */
void color_print(bool bold, int color, const char *fmt, ...);

#endif
