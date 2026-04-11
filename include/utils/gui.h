#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MSG_INFO,
    MSG_SUCCESS,
    MSG_WARNING,
    MSG_ERROR
} msg_type_t;

bool gui_init(void);
void gui_end(void);

void gui_msg(const char *title, const char *msg, msg_type_t type);
bool gui_yes_no(const char *title, const char *msg, bool def);
void gui_progress(const char *title, const char *msg, size_t done, size_t total);

#define GUI_MAX_BARS 10

typedef struct {
    const char *id;
    const char *title;
    const char *msg;
    size_t done;
    size_t total;
    bool active;
} gui_progress_bar_t;

int gui_progress_add(const char *id, const char *title, const char *msg, size_t total);
void gui_progress_update(const char *id, size_t done, size_t total);
void gui_progress_remove(const char *id);
void gui_progress_draw(void);

#endif