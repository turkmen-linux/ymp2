#include <stdio.h>

#include <core/ymp.h>
#include <utils/gui.h>

static int dialog_main(char** args) {
    if(ymp_gui_yes_no(args[0], args[1], false)){
        return 0;
    }
    return 1;
}

void dialog_init(OperationManager* manager){
    Operation op;
    op.name = "dialog";
    op.alias = "ask";
    op.description = "Dialog";
    op.min_args = 2;
    op.help = NULL;
    op.call = (callback)dialog_main;
    operation_register(manager, op);
}
