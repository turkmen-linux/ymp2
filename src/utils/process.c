#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/logger.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utils/error.h>
#include <utils/string.h>

visible size_t get_epoch() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}
static bool locked = false;
visible void single_instance() {
    if (locked) {
        return;
    }
    int pid_file = open("/tmp/ymp.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    locked = true;
    if (rc) {
        if (EWOULDBLOCK == errno) {
            close(pid_file);
            error_add("Another ymp instance is already running");
            exit(31);
        }
    }
}
visible char *which(char *cmd) {
    debug("%s\n", cmd);
    char *fullPath = getenv("PATH");

    struct stat buffer;
    const char *fileOrDirectory = cmd;
    char *fullfilename = calloc(1024, sizeof(char));
    if (!fullfilename) {
        return NULL;
    }

    const char *token = strtok(fullPath, ":");

    /* walk through other tokens */
    while (token != NULL) {
        sprintf(fullfilename, "%s/%s", token, fileOrDirectory);
        int exists = stat(fullfilename, &buffer);
        if (exists == 0 && (S_IFREG & buffer.st_mode)) {
            return (char *) fullfilename;
        }

        token = strtok(NULL, ":"); /* next token */
    }
    free(fullfilename);
    return strdup(cmd);
}

extern char **environ;
visible int run_args(char *args[]) {
    pid_t pid = fork();
    int status = 0;
    char *tmp = join(" ", args);
    debug("%s\n", tmp);
    free(tmp);
    if (pid == 0) {
        execv(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, &status, 0);
    }
    return status;
}
