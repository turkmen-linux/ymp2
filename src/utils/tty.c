#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <termios.h>

#include "utils/tty.h"

static struct termios orig_termios;

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void enable_raw_mode(void) {
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &orig_termios) < 0) {
        perror("tcgetattr");
        exit(1);
    }
    raw = orig_termios;
    cfmakeraw(&raw);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0) {
        perror("tcsetattr");
        exit(1);
    }
}
int setup_raw_mode() {
    struct termios term;
    if (tcgetattr(STDIN_FILENO, &term) < 0) {
        perror("tcgetattr");
        return -1;
    }
    cfmakeraw(&term);
    cfsetispeed(&term, B115200);
    cfsetospeed(&term, B115200);
    term.c_iflag |= ICRNL | IXON;
    term.c_oflag |= OPOST | ONLCR;
    term.c_lflag |= (ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOKE);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0) {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}