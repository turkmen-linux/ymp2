#include <string.h>

#include <ncurses.h>

#include <core/ymp.h>

#include <utils/gui.h>

extern WINDOW *win;
extern int win_w;
extern int win_h;
extern pthread_mutex_t gui_mutex;
extern gui_display_t current_display;
static char *saved_title;
static char *saved_msg;
msg_type_t saved_msg_type;

extern void center_window(int w, int h);
extern void draw_text(const char *text, int y_start, bool center);

#define count_line(A) do {\
  for(size_t i=0; A[i]; i++) {\
      if (A[i] == '\n'){ \
            h++; \
        } \
  } \
} while(0)


visible void gui_message_draw() {
    clear();
    refresh();

    int w = 50, h = 10;
    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);

    count_line(saved_title);
    int hh = h;
    count_line(saved_msg);

    if (w > screen_w)
        w = screen_w - 4;
    if (h > screen_h)
        h = screen_h - 4;

    center_window(w, h);

    char type_ch = ' ';
    short color_pair = 2;
    switch (saved_msg_type) {
    case MSG_ERROR:
        type_ch = 'E';
        color_pair = 3;
        break;
    case MSG_SUCCESS:
        type_ch = 'X';
        color_pair = 4;
        break;
    case MSG_WARNING:
        type_ch = '!';
        color_pair = 5;
        break;
    case MSG_INFO:
    default:
        type_ch = 'i';
        color_pair = 6;
        break;
    }

    wbkgd(win, COLOR_PAIR(color_pair) | A_BOLD);
    box(win, 0, 0);

    if (saved_title) {
        wattron(win, A_BOLD);
        char title[strlen(saved_title)+4];
        snprintf(title, sizeof(title), "[%c] %s", type_ch, saved_title);
        draw_text(title, 1, false);
        wattroff(win, A_BOLD);
    }

    if (saved_msg) {
        draw_text(saved_msg, hh - 8, true);
    }

    mvwaddstr(win, h - 3, win_w / 2 - 6, "[ Press Enter ]");
    wrefresh(win);
}

visible void gui_msg(const char *title, const char *msg, msg_type_t type) {
    gui_init();
    saved_title = (char *)title;
    saved_msg = (char *)msg;
    saved_msg_type = type;
    current_display = GUI_DISPLAY_MESSAGE;

    gui_message_draw();
    wgetch(win);
    current_display = GUI_DISPLAY_NONE;
    gui_end();
}
