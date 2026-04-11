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

bool ymp_gui_init(void);
void ymp_gui_end(void);

void ymp_gui_msg(const char *title, const char *msg, msg_type_t type);
bool ymp_gui_yes_no(const char *title, const char *msg, bool def);
void ymp_gui_progress(const char *title, const char *msg, int done, int total);

void ymp_gui_downloads_init(void);
void ymp_gui_downloads_add(const char* url, const char* path);
void ymp_gui_downloads_start(void);
void ymp_gui_downloads_draw(void);
bool ymp_gui_downloads_done(void);
void ymp_gui_downloads_end(void);

#endif