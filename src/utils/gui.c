#include <signal.h>

#include <string.h>
#include <stdlib.h>

#include <ncurses.h>


static const char* title;
static const char *message;
static void draw_yes_no_dialog(){
    // Get terminal size
    int x, y;
    getmaxyx(stdscr, y, x);

    // Display the message
    char* title_copy = strdup(title);
    char *line = strtok(title_copy, "\n");
    size_t i=0;
    while (line != NULL) {
        mvprintw(y /2 + i , (x - strlen(line)) / 2, "%s", line);
        i++;
        line = strtok(NULL, "\n");
    }
    mvprintw(y /2 + i+1 , (x - strlen(message)) / 2, "%s (y/n)", message);

    // Refresh the screen to show the message
    refresh();
    free(title_copy);

}

bool yes_no_dialog(const char* ftitle, const char *fmessage) {
    title = ftitle;
    message = fmessage;
    initscr();
    noecho();
    cbreak();
    draw_yes_no_dialog();

    // Main input loop
    bool ret = false;
    while (1) {
        char choice = getch();

        if (choice == 'y' || choice == 'Y') {
            ret = true;
            break;
        } else if (choice == 'n' || choice == 'N') {
            ret = false;
            break;
        } else {
            refresh();
        }
    }
    endwin();
    return ret;
}
