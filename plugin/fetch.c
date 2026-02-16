#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

#include <core/ymp.h>
#include <core/operations.h>

#include <utils/fetcher.h>

static VariableManager* vars;

static int fetch_fn(void** args){
    const char* target = variable_get_value(vars, "target");
    char **links = (char**)args;

    if(!target){
        char curdir[PATH_MAX];
        if (getcwd(curdir, sizeof(curdir)) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        target = curdir;
    }
    for(size_t i=0; links[i]; i++){
        char target_file[PATH_MAX+strlen(links[i])+1];
        strcpy(target_file, target);
        strcat(target_file, "/");
        strcat(target_file, basename(links[i]));
        if(!fetch(links[i], target_file)){
            return 1;
        }
    }
    return 0;

}

visible void plugin_init(Ymp* ymp){
    vars = ymp->variables;
    Operation op;
    op.name = "fetch";
    op.alias = "dl:download:wget";
    op.help = NULL;
    op.min_args = 1;
    op.call = (callback) fetch_fn;
    operation_register(ymp->manager, op);
}
