#include <stdlib.h>
#include <stdio.h>
#include <core/ymp.h>
#include <core/logger.h>
#include <core/interpreter.h>

#include <utils/string.h>
#include <utils/error.h>
#include <utils/file.h>

#include <config.h>

visible int ymp_main(int argc, char** argv){
    Ymp* ymp = ymp_init(); // Create ymp context
    if(argc > 1) {
        bool show_ver = false;
        for(int i= 0; argv[i]; i++){
            if(iseq(argv[i], "--version")){
                print("YMP    : %serli ve %silli %sackage manager\n",
                    colorize(RED,"Y"), colorize(RED,"M"), colorize(RED,"P"));
                print("Version: %s\n", VERSION);
                show_ver = true;
            } else if(show_ver && iseq(argv[i], "--flag")) {
                print("\n%s\n", readfile(":/flag.txt"));
            }
        }
        if(show_ver){
            return 0;
        }
        if(isfile(argv[1])){
            return run_script(readfile(argv[1]));
        }
        ymp_add(ymp, argv[1], parse_args(argv+2));
    } else {
        char* err_msg = build_string("No command given.\nRun %s for more information about usage.\n", colorize(RED, "ymp help"));
        error_add(err_msg);
        free(err_msg);
    }
    if(getenv("DEBUG") != NULL){
        logger_set_status(DEBUG, true);
    }
    error(1);
    int status = ymp_run(ymp);
    ymp_unref(ymp);
    return status;
}
