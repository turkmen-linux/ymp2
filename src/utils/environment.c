#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <utils/environment.h>

extern char **environ;
static char **environ_save;
visible void clear_env(){
    char *path = strdup(getenv("PATH"));
    int len = 0;
    while(environ[len]){
        len++;
        environ[len] = NULL;
    }
    setenv("PATH", path, 1);
    free(path);
}

visible void save_env(){
    int len = 0;
    while(environ[len]){
        len++;
    }
    char **ret = malloc((len+1)*sizeof(char*));
    if(!ret){
        return;
    }
    len = 0;
    while(environ[len]){
        ret[len] = strdup(environ[len]);
        len++;
    }
    ret[len] = NULL;
    environ_save = ret;
}

visible void restore_env(){
    clear_env();
    environ = environ_save;
}
