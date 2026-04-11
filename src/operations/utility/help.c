#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <core/operations.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/color.h>

extern Ymp* global;

static void help_print(Operation op){
    if(op.alias != NULL){
        color_print(BOLD, COLOR_CYAN, "%s:", "Aliases");
        color_print(NORMAL, COLOR_DEFAULT, "%s\n", op.alias);
    }
    color_print(BOLD, COLOR_CYAN, "%s: ", "Usage");
    color_print(NORMAL, COLOR_GREEN, "ymp %s [OPTION]... [ARGS]...\n", op.name);
    color_print(NORMAL, COLOR_DEFAULT, "%s\n", op.description);
    if(op.help == NULL){
        return;
    }
    color_print(BOLD, COLOR_CYAN, "%s:\n", "Options");
    for(size_t i=0; i < op.help->cur; i++){
        color_print(NORMAL, COLOR_YELLOW, "  %s\n", op.help->parameters[i]);
    }
}

static int help_op(char* name){
    OperationManager *manager = global->manager;
    for(size_t i=0; i < manager->length; i++){
        Operation op = manager->operations[i];
        if(iseq(name, op.name)){
            help_print(op);
        }
    }
    return 0;
}

static int help_main(char** args){
    int cnt = 0;
    for(size_t i=0; args[i]; i++) {
        help_op(args[i]);
        cnt++;
    }
    if(cnt){
        return 0;
    }
    OperationManager *manager = global->manager;
    for(size_t i=0; i < manager->length; i++){
        Operation op = manager->operations[i];
        color_print(BOLD, COLOR_CYAN, "%s", op.name);
        color_print(NORMAL, COLOR_DEFAULT, "  :  %s\n", op.description);
    }
    return 0;
}

void help_init(OperationManager* manager){
    Operation op;
    op.name = "help";
    op.description = "Print help message";
    op.alias = "h";
    op.min_args = 0;
    op.help = NULL;
    op.call = (callback)help_main;
    operation_register(manager, op);
}