#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <ncurses.h>
#include <panel.h>

#include <core/ymp.h>

#include <utils/gui.h>

WINDOW *win = NULL;
int win_w = 0, win_h = 0;
bool initialized = false;
pthread_mutex_t gui_mutex = PTHREAD_MUTEX_INITIALIZER;

gui_display_t current_display = GUI_DISPLAY_NONE;


extern void gui_message_draw();
extern void gui_yesno_draw();

void gui_force_update(void);

static volatile sig_atomic_t resize_pending = 0;

static void handle_resize(int sig) {
    (void)sig;
    resize_pending = 1;
    if(current_display != GUI_DISPLAY_PROGRESS){
        gui_handle_resize();
    }
}

visible void gui_handle_resize(void) {
    if (resize_pending) {
        resize_pending = 0;
        endwin();
        clear();
        refresh();
        gui_force_update();
        ungetch(KEY_RESIZE);
    }
}

bool init_ncurses(void) {
    pthread_mutex_lock(&gui_mutex);
    if (initialized) {
        pthread_mutex_unlock(&gui_mutex);
        return true;
    }

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
    pthread_mutex_unlock(&gui_mutex);
    return true;
}

void center_window(int w, int h) {
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

void draw_text(const char *text, int y_start, bool center) {
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

void cleanup(void) {
    pthread_mutex_lock(&gui_mutex);
    if (win) {
        delwin(win);
        win = NULL;
    }
    if (initialized) {
        endwin();
        initialized = false;
    }
    current_display = GUI_DISPLAY_NONE;

    pthread_mutex_unlock(&gui_mutex);
}

visible bool gui_init(void) {
    return init_ncurses();
}

visible void gui_end(void) {
    cleanup();
}

void gui_force_update(void) {
    gui_handle_resize();
    switch (current_display) {
    case GUI_DISPLAY_MESSAGE:
        gui_message_draw();
        break;
    case GUI_DISPLAY_YESNO:
        gui_yesno_draw();
        break;
    case GUI_DISPLAY_PROGRESS:
    case GUI_DISPLAY_NONE:
        break;
    }
}
