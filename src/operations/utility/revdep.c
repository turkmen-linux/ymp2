#include <core/ymp.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/jobs.h>
#include <utils/process.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int pkgconf_callback(void* args){
    char* name = (char*)args;
    pid_t pid = fork();
    if(pid == 0){
        FILE *fp = freopen("/dev/null", "w", stdout);
        if (fp == NULL) {
           perror("freopen");
           exit(1);
        }
        char* cmd[] = {which("pkgconf"), name, "--libs", "--cflags", NULL};
        exit(run_args(cmd));
    }
    int status;
    waitpid(pid, &status, 0);
    return status;
}

static int pkgconf_check(){
    char *dirs[] = {
        "/usr/lib/pkgconfig",
        "/lib/pkgconfig",
        "/usr/share/pkgconfig",
        NULL
    };
    for(size_t i=0; dirs[i]; i++){
        char** files = listdir(dirs[i]);
        jobs* job = jobs_new();
        for(size_t j=0; files[j]; j++){
            if(files[j][0] == '.'){
                continue;
            }
            files[j][strlen(files[j])-3] = '\0';
            jobs_add(job, (callback)pkgconf_callback, files[j], NULL);
        }
        jobs_run(job);
        jobs_unref(job);
        for(size_t j=0; files[j]; j++){
            free(files[j]);
        }
        free(files);
    }
    return 0;
}

static int revdep_main(void** args) {
    (void)args;
    if(get_bool("pkgconfig")){
        pkgconf_check();
    }
    return 0;
}

void revdep_init(OperationManager* manager){
    Operation op;
    op.name = "revdep-rebuild";
    op.alias = "rbd:cr";
    op.description = "Check library for broken links.";
    op.min_args = 0;
    op.help = NULL;
    op.call = (callback)revdep_main;
    operation_register(manager, op);
}