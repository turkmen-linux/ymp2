#include <string.h>
#include <stdlib.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/process.h>

#include <core/ymp.h>
#include <core/variable.h>
#include <core/logger.h>

#include <config.h>

static int key_add(char** args){
    int status = 0;
    char* name = get_value("name");
    char* destdir = get_value("DESTDIR");
    if(strlen(name) == 0){
        print("Key name is not defined!\n");
        return 1;
    }
    char* file = args[0];
    char* dest = build_string("%s/%s/gpg/%s.asc", destdir, STORAGE, name);
    if(isfile(file)){
        copy_file(file, dest);
        char* cmd[] = {"gpg", "--dearmor", dest, NULL};
        status = run_args(cmd);
        unlink(dest);
    }
    free(dest);
    return status;
}

static int key_remove(char** args){
    int status = 0;
    char* destdir = get_value("DESTDIR");
    for(size_t i=0; args[i]; i++){
        char* dest = build_string("%s/%s/gpg/%s.gpg", destdir, STORAGE, args[i]);
        if(isfile(dest)){
            unlink(dest);
        } else {
            warning("Key not found: %s\n", args[i]);
        }
    }
    return status;
}

static int key_main(char** args){
    if(get_bool("add")){
        return key_add(args);
    } else if(get_bool("remove")){
        return key_remove(args);
    }
    return 0;
}

void key_init(OperationManager *manager){
    Operation op;
    op.name = "key";
    op.alias = "k";
    op.description = "Ymp key manager";
    op.min_args = 0;
    op.help = NULL;
    op.call = (callback)key_main;
    operation_register(manager, op);
}