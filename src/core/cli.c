#include <stdlib.h>
#include <stdio.h>
#include <core/ymp.h>
#include <core/logger.h>
#include <core/interpreter.h>

#include <utils/string.h>
#include <utils/error.h>
#include <utils/file.h>
#include <utils/color.h>

#include <config.h>

#define red "\033[31;1m"
#define nop "\033[0m"

visible int ymp_main(int argc, char** argv){
    Ymp* ymp = ymp_init(); // Create ymp context
    if(argc > 1) {
        bool show_ver = false;
        for(int i= 0; argv[i]; i++){
            if(iseq(argv[i], "--version")){
                color_print(BOLD, COLOR_RED, "YMP");
                print("    : %serli ve %silli %sackage manager\n",
                    red "Y" nop, red "M" nop , red "P" nop);
                print(_("Version: %s\n"), VERSION);
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
        print(_("No command given.\n"));
        print(_("Run %s for more information about usage.\n"), red "ymp help" nop);
        ymp_unref(ymp);
        return 1;
    }
    if(getenv("DEBUG") != NULL){
        logger_set_status(DEBUG, true);
    }
    int status = ymp_run(ymp);
    ymp_unref(ymp);
    return status;
}
