#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <ncurses.h>
#include <panel.h>

#include <core/ymp.h>

#include <utils/gui.h>
#include <utils/fetcher.h>

typedef struct {
    const char *id;
    const char *title;
    const char *msg;
    size_t done;
    size_t total;
    bool active;
} gui_progress_bar_t;

static WINDOW *win = NULL;
static int win_w = 0, win_h = 0;
static bool initialized = false;
static gui_progress_bar_t progress_bars[GUI_MAX_BARS];
static int progress_bar_count = 0;
static pthread_mutex_t gui_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool init_ncurses(void);
static void format_size(char *buf, size_t buf_len, size_t bytes);


static void cleanup(void) {
    pthread_mutex_lock(&gui_mutex);
    if (win) {
        delwin(win);
        win = NULL;
    }
    if (initialized) {
        endwin();
        initialized = false;
    }
    for (int i = 0; i < GUI_MAX_BARS; i++) {
        free((void *)progress_bars[i].id);
        free((void *)progress_bars[i].title);
        free((void *)progress_bars[i].msg);
        progress_bars[i].active = false;
        progress_bars[i].id = NULL;
        progress_bars[i].title = NULL;
        progress_bars[i].msg = NULL;
    }
    progress_bar_count = 0;
    pthread_mutex_unlock(&gui_mutex);
}

static void handle_resize(int sig) {
    (void)sig;
    if (win) {
        delwin(win);
        win = NULL;
    }
    if (initialized) {
        endwin();
        initialized = false;
    }
    init_ncurses();
    gui_progress_draw();
}

static bool init_ncurses(void) {
    pthread_mutex_lock(&gui_mutex);
    if (initialized) {
        pthread_mutex_unlock(&gui_mutex);
        return true;
    }

    signal(SIGWINCH, handle_resize);

    for (int i = 0; i < GUI_MAX_BARS; i++) {
        progress_bars[i].active = false;
        progress_bars[i].id = NULL;
        progress_bars[i].title = NULL;
        progress_bars[i].msg = NULL;
        progress_bars[i].done = 0;
        progress_bars[i].total = 0;
    }
    progress_bar_count = 0;

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
    pthread_mutex_unlock(&gui_mutex);
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

visible bool gui_init(void) {
    return init_ncurses();
}

visible void gui_end(void) {
    cleanup();
    pthread_mutex_destroy(&gui_mutex);
}

visible void gui_msg(const char *title, const char *msg, msg_type_t type) {
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

visible bool gui_yes_no(const char *title, const char *msg, bool def) {
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

        mvwprintw(win, h - 4, 2, _("Select: "));
        if (sel == 0) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, h - 4, 10, _(" Yes "));
        if (sel == 0) {
            wattroff(win, A_REVERSE);
        }
        mvwprintw(win, h - 4, 16, "  ");

        if (sel == 1) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, h - 4, 18, _(" No "));
        if (sel == 1) {
            wattroff(win, A_REVERSE);
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

static void draw_progress_bar(WINDOW *w, int y, const char *title, const char *msg, size_t done, size_t total) {
    int w_w, w_h;
    getmaxyx(w, w_h, w_w);
    (void)w_h;

    if (title) {
        wattron(w, A_BOLD);
        mvwprintw(w, y, 2, "%s", title);
        wattroff(w, A_BOLD);
    }

    if (msg) {
        mvwprintw(w, y + 1, 2, "%s", msg);
    }

    int bar_w = w_w - 28;
    int filled = 0;
    int pct = 0;
    if (total > 0) {
        pct = (done * 100) / total;
        filled = (pct * bar_w) / 100;
        if (filled > bar_w)
            filled = bar_w;
    }

    mvwprintw(w, y + 2, 2, "[");
    for (int i = 0; i < bar_w; i++) {
        if (i < filled) {
            waddch(w, '=' | A_BOLD | COLOR_PAIR(4));
        } else if (i == filled) {
            waddch(w, '>' | A_BOLD);
        } else {
            waddch(w, '-');
        }
    }
    waddstr(w, "]");

    char done_str[16], total_str[16];
    format_size(done_str, sizeof(done_str), done);
    format_size(total_str, sizeof(total_str), total);
    mvwprintw(w, y + 2, w_w - 26, "%s/%s", done_str, total_str);

    if (total > 0) {
        mvwprintw(w, y + 2, w_w - 9, "%3d%%", pct);
    } else {
        mvwprintw(w, y + 2, w_w - 9, "   ?");
    }
}

visible int gui_progress_add(const char *id, const char *title, const char *msg, size_t total) {
    if (!id)
        return -1;

    pthread_mutex_lock(&gui_mutex);
    if (progress_bar_count >= GUI_MAX_BARS) {
        pthread_mutex_unlock(&gui_mutex);
        return -1;
    }

    if (total == 0)
        total = 100;

    for (int i = 0; i < GUI_MAX_BARS; i++) {
        if (!progress_bars[i].active) {
            progress_bars[i].id = id ? strdup(id) : NULL;
            progress_bars[i].title = title ? strdup(title) : NULL;
            progress_bars[i].msg = msg ? strdup(msg) : NULL;
            progress_bars[i].done = 0;
            progress_bars[i].total = total;
            progress_bars[i].active = true;
            progress_bar_count++;
            pthread_mutex_unlock(&gui_mutex);
            return i;
        }
    }
    pthread_mutex_unlock(&gui_mutex);
    return -1;
}

visible void gui_progress_update(const char *id, size_t done, size_t total) {
    if (!id)
        return;

    pthread_mutex_lock(&gui_mutex);
    for (int i = 0; i < GUI_MAX_BARS; i++) {
        if (progress_bars[i].active && progress_bars[i].id && strcmp(progress_bars[i].id, id) == 0) {
            progress_bars[i].done = done;
            progress_bars[i].total = total;
            pthread_mutex_unlock(&gui_mutex);
            return;
        }
    }
    pthread_mutex_unlock(&gui_mutex);
}

static void format_size(char *buf, size_t buf_len, size_t bytes) {
    if (bytes >= 1024 * 1024 * 1024) {
        snprintf(buf, buf_len+1, "%zu.%zuGB", bytes / (1024 * 1024 * 1024),
                (bytes % (1024 * 1024 * 1024)) / (1024 * 1024 * 102));
    } else if (bytes >= 1024 * 1024) {
        snprintf(buf, buf_len+1, "%zu.%zuMB", bytes / (1024 * 1024),
                (bytes % (1024 * 1024)) / (1024 * 102));
    } else if (bytes >= 1024) {
        snprintf(buf, buf_len+1, "%zu.%zuKB", bytes / 1024,
                (bytes % 1024) / 102);
    } else {
        snprintf(buf, buf_len+1, "%zuB", bytes);
    }
}

visible void gui_progress_remove(const char *id) {
    if (!id)
        return;

    pthread_mutex_lock(&gui_mutex);
    for (int i = 0; i < GUI_MAX_BARS; i++) {
        if (progress_bars[i].active && progress_bars[i].id && strcmp(progress_bars[i].id, id) == 0) {
            free((void *)progress_bars[i].id);
            free((void *)progress_bars[i].title);
            free((void *)progress_bars[i].msg);
            progress_bars[i].active = false;
            progress_bars[i].id = NULL;
            progress_bars[i].title = NULL;
            progress_bars[i].msg = NULL;
            progress_bar_count--;
            pthread_mutex_unlock(&gui_mutex);
            clear();
            gui_progress_draw();
            return;
        }
    }
    pthread_mutex_unlock(&gui_mutex);
}

visible void gui_progress_draw(void) {
    if (!initialized) {
        if (!init_ncurses())
            return;
    }

    pthread_mutex_lock(&gui_mutex);

    //clear();
    refresh();

    int active_count = 0;
    for (int i = 0; i < GUI_MAX_BARS; i++) {
        if (progress_bars[i].active)
            active_count++;
    }

    if (active_count == 0) {
        pthread_mutex_unlock(&gui_mutex);
        return;
    }

    int bar_h = 4;
    int h = active_count * bar_h + 4;
    int w = 60;

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

        int y = 1;
        for (int i = 0; i < GUI_MAX_BARS; i++) {
            if (progress_bars[i].active) {
                draw_progress_bar(win, y, progress_bars[i].title, progress_bars[i].msg,
                               progress_bars[i].done, progress_bars[i].total);
                y += bar_h;
            }
        }

        wrefresh(win);
    }
    pthread_mutex_unlock(&gui_mutex);
}

