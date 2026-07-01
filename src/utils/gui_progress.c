#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include <core/ymp.h>

#include <utils/gui.h>

extern WINDOW *win;
extern gui_progress_bar_t progress_bars[];
static int progress_bar_count = 0;
extern pthread_mutex_t gui_mutex;
extern gui_display_t current_display;

extern void center_window(int w, int h);

void gui_progress_draw(void);

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
    if (progress_bar_count == 0){
        gui_init();
    }
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
    if (progress_bar_count == 0){
        gui_end();
    }
    pthread_mutex_unlock(&gui_mutex);
}

visible void gui_progress_draw(void) {
    pthread_mutex_lock(&gui_mutex);

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
    current_display = GUI_DISPLAY_PROGRESS;
    pthread_mutex_unlock(&gui_mutex);
}
