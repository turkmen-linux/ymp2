#define _GNU_SOURCE
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>

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
