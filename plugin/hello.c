#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <core/operations.h>

static int hello_fn(void** args){
    (void)args;
    print(_("Hello World\n"));
    return 0;
}

visible void plugin_init(Ymp* ymp){
    // too simple plugin
    if(getenv("hello")){
        Operation op;
        op.name = "hello";
        op.description = _("Simple hello world plugin");
        op.alias = NULL;
        op.help = NULL;
        op.min_args = 0;
        op.call = (callback) hello_fn;
        operation_register(ymp->manager, op);
    }
}
