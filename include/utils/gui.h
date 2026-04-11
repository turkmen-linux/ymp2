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
void gui_progress(const char *title, const char *msg, int done, int total);

#endif