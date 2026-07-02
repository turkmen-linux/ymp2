#include <string.h>

#include <ncurses.h>

#include <core/ymp.h>

#include <utils/gui.h>

extern WINDOW *win;
extern int win_w;
extern int win_h;
extern gui_display_t current_display;

static char *saved_title;
static char *saved_msg;
static bool saved_yesno;


extern void center_window(int w, int h);
extern void draw_text(const char *text, int y_start, bool center);

#define count_line(A) do {\
  for(size_t i=0; A[i]; i++) {\
      if (A[i] == '\n'){ \
            h++; \
        } \
  } \
} while(0)

visible void gui_yesno_draw() {
    clear();
    refresh();

    size_t title_len = saved_title ? strlen(saved_title) : 0;
    size_t msg_len = saved_msg ? strlen(saved_msg) : 0;
    size_t max_len = title_len > msg_len ? title_len : msg_len;

    int w = (max_len > 40 ? max_len + 12 : 52);
    int h = 12;
    count_line(saved_title);
    int hh = h;
    count_line(saved_msg);

    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);
    if (w > screen_w)
        w = screen_w - 4;
    if (h > screen_h)
        h = screen_h - 4;

    center_window(w, h);

    wbkgd(win, COLOR_PAIR(2));
    box(win, 0, 0);

    if (saved_title) {
        wattron(win, A_BOLD);
        draw_text(saved_title, 0, false);
        wattroff(win, A_BOLD);
    }

    if (saved_msg) {
        draw_text(saved_msg, hh - 8, true);
    }

    mvwprintw(win, h - 4, 2, _("Select: "));
    if (saved_yesno) {
        wattron(win, A_REVERSE);
    }
    mvwprintw(win, h - 4, 10, _(" Yes "));
    if (saved_yesno) {
        wattroff(win, A_REVERSE);
    }
    mvwprintw(win, h - 4, 16, "  ");

    if (!saved_yesno) {
        wattron(win, A_REVERSE);
    }
    mvwprintw(win, h - 4, 18, _(" No "));
    if (!saved_yesno) {
        wattroff(win, A_REVERSE);
    }

    wrefresh(win);
}

visible bool gui_yes_no(const char *title, const char *msg, bool def) {
    gui_init();
    saved_title = (char *)title;
    saved_msg = (char *)msg;
    saved_yesno = def;
    current_display = GUI_DISPLAY_YESNO;

    bool ret = def;
    int sel = def ? 0 : 1;
    int ch;

    gui_yesno_draw();

    do {
        ch = wgetch(win);

        if (ch == KEY_LEFT || ch == 'h') {
            sel = 0;
            saved_yesno = true;
            gui_yesno_draw();
        } else if (ch == KEY_RIGHT || ch == 'l') {
            sel = 1;
            saved_yesno = false;
            gui_yesno_draw();
        } else if (ch == '\n' || ch == '\r') {
            ret = (sel == 0);
            break;
        } else if (ch == 'y' || ch == 'Y') {
            ret = true;
            break;
        } else if (ch == 'n' || ch == 'N') {
            ret = false;
            break;
        } else if (ch == 27 || ch == 'q') {
            break;
        }
    } while (1);
    current_display = GUI_DISPLAY_NONE;
    gui_end();
    return ret;
}
