#include <stdio.h>

#include <core/ymp.h>
#include <utils/gui.h>

#include <string.h>

static int dialog_main(char** args) {
    if(get_bool("yes-no")){
        if(gui_yes_no(args[0], args[1], false)){
            return 0;
        }
        return 1;
    } else if (get_bool("message")) {
        gui_msg(args[0], args[1], MSG_INFO);
    }
    return 0;
}

void dialog_init(OperationManager* manager){
    Operation op;
    op.name = "dialog";
    op.alias = "ask";
    op.description = _("Dialog");
    op.min_args = 2;
    op.help = NULL;
    op.call = (callback)dialog_main;
    operation_register(manager, op);
}
