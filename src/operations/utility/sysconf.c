#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <unistd.h>

#include <core/ymp.h>
#include <core/logger.h>

#include <utils/process.h>
#include <utils/environment.h>
#include <utils/file.h>
#include <utils/string.h>

static int sysconf_main(char** args){
    (void)args;
    int rc = 0;
    const char* destdir = get_value("DESTDIR");
    pid_t pid = fork();
    if(pid == 0){
        char* sysconfdir= build_string("%s/etc/sysconf.d/", destdir);
        clear_env();
        setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
        setenv("OPERATION", get_value ("OPERATION"), 1);
        char** sc = listdir(sysconfdir);
        if(strlen(destdir) > 0 && strcmp(destdir, "/") != 0){
            rc = chroot(destdir);
            if(rc < 0){
                warning("Failed to chroot: %s\n", destdir);
                exit(1);
            }
        }
        char trigger[PATH_MAX];
        for(size_t i=0; sc[i]; i++){
            strcpy(trigger, "/etc/sysconf.d/");
            strcat(trigger, sc[i]);
            debug("%s\n", trigger);
            char* args[] = {"/bin/sh", trigger, NULL};
            rc = run_args(args);
            if(rc != 0){
                exit(rc);
            }
        }
        exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    return status;
}

void sysconf_init(OperationManager *manager){
    Operation op;
    op.name = "sysconf";
    op.alias = "sc";
    op.description = "Trigger sysconf operations";
    op.min_args = 0;
    op.help = NULL;
    op.call = (callback)sysconf_main;
    operation_register(manager, op);
}