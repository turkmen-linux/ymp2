#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <ncurses.h>
#include <panel.h>

#include <utils/gui.h>
#include <utils/fetcher.h>

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

typedef struct {
    char* url;
    char* path;
    size_t downloaded;
    size_t total;
    bool done;
    bool error;
    char* error_msg;
} FetchItem;

static FetchItem* fetch_items = NULL;
static size_t fetch_count = 0;
static size_t fetch_capacity = 0;
static pthread_mutex_t fetch_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool fetching = false;

void ymp_gui_downloads_init(void) {
    pthread_mutex_lock(&fetch_mutex);
    if (fetch_items) {
        for (size_t i = 0; i < fetch_count; i++) {
            free(fetch_items[i].url);
            free(fetch_items[i].path);
            free(fetch_items[i].error_msg);
        }
        free(fetch_items);
    }
    fetch_items = NULL;
    fetch_count = 0;
    fetch_capacity = 0;
    pthread_mutex_unlock(&fetch_mutex);
}

static void download_progress_cb(const char* url, size_t downloaded, size_t total, void* userdata) {
    (void)userdata;
    pthread_mutex_lock(&fetch_mutex);
    for (size_t i = 0; i < fetch_count; i++) {
        if (fetch_items[i].url && strcmp(fetch_items[i].url, url) == 0) {
            fetch_items[i].downloaded = downloaded;
            fetch_items[i].total = total;
            break;
        }
    }
    pthread_mutex_unlock(&fetch_mutex);
}

void ymp_gui_downloads_add(const char* url, const char* path) {
    pthread_mutex_lock(&fetch_mutex);
    if (fetch_count >= fetch_capacity) {
        fetch_capacity = fetch_capacity ? fetch_capacity * 2 : 4;
        fetch_items = realloc(fetch_items, sizeof(FetchItem) * fetch_capacity);
    }
    fetch_items[fetch_count].url = strdup(url);
    fetch_items[fetch_count].path = strdup(path);
    fetch_items[fetch_count].downloaded = 0;
    fetch_items[fetch_count].total = 0;
    fetch_items[fetch_count].done = false;
    fetch_items[fetch_count].error = false;
    fetch_items[fetch_count].error_msg = NULL;
    fetch_count++;
    pthread_mutex_unlock(&fetch_mutex);
}

static void* download_thread(void* arg) {
    (void)arg;
    pthread_mutex_lock(&fetch_mutex);
    fetching = true;
    pthread_mutex_unlock(&fetch_mutex);

    for (size_t i = 0; i < fetch_count; i++) {
        if (!fetching) break;
        fetch_items[i].done = fetch_with_progress(fetch_items[i].url, fetch_items[i].path, download_progress_cb, NULL);
        if (!fetch_items[i].done) {
            fetch_items[i].error = true;
            fetch_items[i].error_msg = strdup("Download failed");
        }
    }

    pthread_mutex_lock(&fetch_mutex);
    fetching = false;
    pthread_mutex_unlock(&fetch_mutex);
    return NULL;
}

void ymp_gui_downloads_start(void) {
    pthread_t tid;
    pthread_create(&tid, NULL, download_thread, NULL);
    pthread_detach(tid);
}

void ymp_gui_downloads_draw(void) {
    if (!initialized) {
        if (!init_ncurses())
            return;
    }

    clear();
    refresh();

    pthread_mutex_lock(&fetch_mutex);
    size_t count = fetch_count;
    pthread_mutex_unlock(&fetch_mutex);

    int screen_w, screen_h;
    getmaxyx(stdscr, screen_h, screen_w);

    int w = screen_w - 4;
    int h = count > 0 ? count + 4 : 8;
    if (h > screen_h - 4)
        h = screen_w - 4;

    center_window(w, h);

    if (!win)
        return;

    wbkgd(win, COLOR_PAIR(2));
    box(win, 0, 0);

    wattron(win, A_BOLD);
    mvwprintw(win, 1, 2, "Downloads");
    wattroff(win, A_BOLD);

    int bar_w = win_w - 30;
    size_t total_downloaded = 0;
    size_t total_size = 0;

    for (size_t i = 0; i < count; i++) {
        int y = 3 + i;

        pthread_mutex_lock(&fetch_mutex);
        char* url = fetch_items[i].url;
        size_t downloaded = fetch_items[i].downloaded;
        size_t total = fetch_items[i].total;
        bool done = fetch_items[i].done;
        bool error = fetch_items[i].error;
        pthread_mutex_unlock(&fetch_mutex);

        if (url) {
            char* fname = strrchr(url, '/');
            fname = fname ? fname + 1 : url;

            if (error) {
                wattron(win, COLOR_PAIR(3));
                mvwprintw(win, y, 2, "X %s", fname);
                wattroff(win, COLOR_PAIR(3));
            } else if (done) {
                wattron(win, COLOR_PAIR(4));
                mvwprintw(win, y, 2, "* %s", fname);
                wattroff(win, COLOR_PAIR(4));
            } else {
                mvwprintw(win, y, 2, "o %s", fname);
            }

            int filled = 0;
            if (total > 0) {
                filled = (downloaded * bar_w) / total;
                if (filled > bar_w)
                    filled = bar_w;
            }

            mvwprintw(win, y, win_w - 28, "[");
            for (int j = 0; j < bar_w; j++) {
                if (j < filled) {
                    waddch(win, '=' | A_BOLD | COLOR_PAIR(4));
                } else {
                    waddch(win, '-');
                }
            }
            waddstr(win, "]");

            if (total > 0) {
                int pct = (downloaded * 100) / total;
                mvwprintw(win, y, win_w - 8, "%3d%%", pct);
                total_downloaded += downloaded;
                total_size += total;
            } else {
                mvwprintw(win, y, win_w - 8, "   ");
            }
        }
    }

    if (count > 0) {
        int y = 3 + count;
        mvwprintw(win, y, 2, "Total:");
        size_t total_done = 0;
        for (size_t i = 0; i < count; i++) {
            pthread_mutex_lock(&fetch_mutex);
            if (fetch_items[i].done) total_done++;
            pthread_mutex_unlock(&fetch_mutex);
        }
        mvwprintw(win, y, win_w - 15, "%zu/%zu", total_done, count);
    }

    wrefresh(win);
}

bool ymp_gui_downloads_done(void) {
    pthread_mutex_lock(&fetch_mutex);
    bool done = !fetching;
    pthread_mutex_unlock(&fetch_mutex);
    return done;
}

void ymp_gui_downloads_end(void) {
    pthread_mutex_lock(&fetch_mutex);
    fetching = false;
    pthread_mutex_unlock(&fetch_mutex);

    pthread_mutex_lock(&fetch_mutex);
    if (fetch_items) {
        for (size_t i = 0; i < fetch_count; i++) {
            free(fetch_items[i].url);
            free(fetch_items[i].path);
            free(fetch_items[i].error_msg);
        }
        free(fetch_items);
    }
    fetch_items = NULL;
    fetch_count = 0;
    fetch_capacity = 0;
    pthread_mutex_unlock(&fetch_mutex);
}