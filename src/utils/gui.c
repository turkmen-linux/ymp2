#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include <ncurses.h>
#include <panel.h>

#include <utils/gui.h>

static WINDOW *win = NULL;
static int win_w = 0, win_h = 0;
static bool initialized = false;

static void cleanup(void) {
    if (win) {
        delwin(win);
        win = NULL;
    }
    if (initialized) {
        endwin();
        initialized = false;
    }
}

static void handle_resize(int sig) {
    (void)sig;
    if (win) {
        delwin(win);
        endwin();
        initialized = false;
    }
}

static bool init_ncurses(void) {
    if (initialized)
        return true;

    signal(SIGWINCH, handle_resize);

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_WHITE);
        init_pair(2, COLOR_WHITE, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_YELLOW, COLOR_BLACK);
        init_pair(6, COLOR_BLUE, COLOR_BLACK);
    }

    initialized = true;
    return true;
}

static void center_window(int w, int h) {
    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);

    int x = (screen_w - w) / 2;
    int y = (screen_h - h) / 2;

    if (win) {
        delwin(win);
    }

    win = newwin(h, w, y, x);
    if (!win)
        return;

    win_w = w;
    win_h = h;

    wbkgd(win, COLOR_PAIR(2));
    keypad(win, true);
    box(win, 0, 0);
}

static void draw_text(const char *text, int y_start, bool center) {
    if (!win || !text)
        return;

    char *text_copy = strdup(text);
    if (!text_copy)
        return;

    char *line = strtok(text_copy, "\n");
    int y = y_start;

    while (line && y < win_h - 2) {
        int x = center ? (win_w - strlen(line)) / 2 : 2;
        mvwprintw(win, y, x, "%s", line);
        line = strtok(NULL, "\n");
        y++;
    }

    free(text_copy);
}

bool ymp_gui_init(void) {
    return init_ncurses();
}

void ymp_gui_end(void) {
    cleanup();
}

void ymp_gui_msg(const char *title, const char *msg, msg_type_t type) {
    if (!init_ncurses())
        return;

    clear();
    refresh();

    int w = 50, h = 10;
    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);

    if (w > screen_w)
        w = screen_w - 4;
    if (h > screen_h)
        h = screen_h - 4;

    center_window(w, h);

    char type_ch = ' ';
    short color_pair = 2;
    switch (type) {
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

    if (title) {
        wattron(win, A_BOLD);
        mvwprintw(win, 1, 2, "[%c] %s", type_ch, title);
        wattroff(win, A_BOLD);
    }

    if (msg) {
        draw_text(msg, 3, true);
    }

    mvwaddstr(win, h - 3, win_w / 2 - 6, "[ Press Enter ]");
    wrefresh(win);

    wgetch(win);
    cleanup();
}

bool ymp_gui_yes_no(const char *title, const char *msg, bool def) {
    if (!init_ncurses())
        return def;

    clear();
    refresh();

    size_t title_len = title ? strlen(title) : 0;
    size_t msg_len = msg ? strlen(msg) : 0;
    size_t max_len = title_len > msg_len ? title_len : msg_len;

    int w = (max_len > 40 ? max_len + 12 : 52);
    int h = 12;

    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);
    if (w > screen_w)
        w = screen_w - 4;
    if (h > screen_h)
        h = screen_h - 4;

    center_window(w, h);

    bool ret = def;
    int sel = def ? 0 : 1;
    int ch;

    do {
        wbkgd(win, COLOR_PAIR(2));
        box(win, 0, 0);

        if (title) {
            wattron(win, A_BOLD);
            mvwprintw(win, 1, 2, "%s", title);
            wattroff(win, A_BOLD);
        }

        if (msg) {
            draw_text(msg, 3, true);
        }

        mvwprintw(win, h - 4, 2, "Select: ");
        if (sel == 0) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, h - 4, 10, " Yes ");
        if (sel == 0) {
            wattroff(win, A_REVERSE);
        }
        mvwprintw(win, h - 4, 16, "  ");

        if (sel == 1) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, h - 4, 18, " No ");
        if (sel == 1) {
            wattroff(win, A_REVERSE);
        }

        if (sel == 0) {
            mvwaddstr(win, h - 3, win_w / 2 - 10, "[ Press Enter to confirm ]");
        } else {
            mvwaddstr(win, h - 3, win_w / 2 - 9, "[ Press Enter to deny ]");
        }

        wrefresh(win);

        ch = wgetch(win);

        if (ch == KEY_LEFT || ch == 'h') {
            sel = 0;
        } else if (ch == KEY_RIGHT || ch == 'l') {
            sel = 1;
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

    cleanup();
    return ret;
}

void ymp_gui_progress(const char *title, const char *msg, int done, int total) {
    if (!initialized) {
        if (!init_ncurses())
            return;
    }

    clear();
    refresh();

    int w = 50, h = 8;

    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);
    if (w > screen_w)
        w = screen_w - 4;
    if (h > screen_h)
        h = screen_h - 4;

    center_window(w, h);

    if (win) {
        wbkgd(win, COLOR_PAIR(2));
        box(win, 0, 0);

        if (title) {
            wattron(win, A_BOLD);
            mvwprintw(win, 1, 2, "%s", title);
            wattroff(win, A_BOLD);
        }

        if (msg) {
            mvwprintw(win, 3, 2, "%s", msg);
        }

        int bar_w = win_w - 8;
        int filled = 0;
        if (total > 0) {
            filled = (done * bar_w) / total;
            if (filled > bar_w)
                filled = bar_w;
        }

        mvwprintw(win, 5, 2, "[");
        for (int i = 0; i < bar_w; i++) {
            if (i < filled) {
                waddch(win, '=' | A_BOLD | COLOR_PAIR(4));
            } else if (i == filled) {
                waddch(win, '>' | A_BOLD);
            } else {
                waddch(win, '-');
            }
        }
        waddstr(win, "]");

        if (total > 0) {
            int pct = (done * 100) / total;
            mvwprintw(win, 5, win_w - 10, "%3d%%", pct);
        } else {
            mvwprintw(win, 5, win_w - 10, "   0%%");
        }

        wrefresh(win);
    }
}