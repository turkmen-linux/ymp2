#include <core/ymp.h>

#include <utils/string.h>
#include <utils/file.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int pkgconf_check(){
    char *dirs[] = {
        "/usr/lib/pkgconfig",
        "/lib/pkgconfig",
        "/usr/share/pkgconfig",
        NULL
    };
    for(size_t i=0; dirs[i]; i++){
        char** files = listdir(dirs[i]);
        for(size_t j=0; files[j]; j++){
            files[j][strlen(files[j])-3] = '\0';
            printf("%s\n", files[j]);
            free(files[j]);
        }
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