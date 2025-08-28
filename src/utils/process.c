#define _GNU_SOURCE
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <utils/error.h>

visible size_t get_epoch(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}
static bool locked = false;
visible void single_instance(){
    if(locked){
        return;
    }
    int pid_file = open("/tmp/ymp.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    locked = true;
    if(rc) {
        if(EWOULDBLOCK == errno){
            error_add("Another ymp instance is already running");
            exit(31);
        }
    }
}
visible char* which(char* cmd){
    char* fullPath = getenv("PATH");

    struct stat buffer;
    int exists;
    char* fileOrDirectory = cmd;
    char *fullfilename = calloc(1024, sizeof(char));

    char *token = strtok(fullPath, ":");

    /* walk through other tokens */
    while( token != NULL ){
        sprintf(fullfilename, "%s/%s", token, fileOrDirectory);
        exists = stat( fullfilename, &buffer );
        if ( exists == 0 && ( S_IFREG & buffer.st_mode ) ) {
            return (char*)fullfilename;
        }

        token = strtok(NULL, ":"); /* next token */
    }
    free(fullfilename);
    return (char*)cmd;
}


visible int run_args(char *args[]) {
    pid_t pid = fork();
    int status = 0;
    if (pid == 0) {
        execv(which(args[0]), args);
        perror("execv failed");
        exit(EXIT_FAILURE);
    } else {
        waitpid(pid, &status, 0);
    }
    return status;
}
