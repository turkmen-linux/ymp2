#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <utils/error.h>
#include <core/logger.h>

#include <utils/array.h>
#include <utils/string.h>

typedef struct {
    const char* name;
    size_t line;
} label;

static label *labels;
size_t label_max = 32;
size_t label_cur = 0;


visible char** parse_args(char** args) {

    size_t len = 0;
    size_t offset = 0;

    // find len
    for(len=0; args[len];len++){}

    for(size_t i=0; i<len;i++){
        // check --
        debug("parse: %s\n", args[i]);
        if (strlen(args[i]) > 2 && args[i][0] == '-' && args[i][1] == '-'){
            for(offset=0; args[i][offset] && args[i][offset]!='='; offset++){}
            char var[offset-1];
            strncpy(var, args[i]+2, offset-2);
            var[offset-2]= '\0';
            debug("offset=%lld len=%lld\n", offset, strlen(args[i]));
            // if does not have =
            if (offset >= strlen(args[i])){
                set_value(var, "true");
            } else {
                char val[strlen(args[i]) - offset - 1];
                strncpy(val, args[i]+offset+1, strlen(args[i]) - offset - 1);
                val[strlen(args[i]) - offset -1] = '\0';
                set_value(var, val);
            }
            args[i]=NULL;
        }
    }
    // relocate memory
    size_t removed = 0;
    for(size_t i=0; i<len;i++){
        if (args[i] == NULL){
            removed++;
            continue;
        }
        args[i-removed] = args[i];
    }
    return args;
}

static char** parse_line(const char* line){
    return parse_args(split(strip((char*)line), " "));
}
visible int run_script(const char* script){
    char** lines = split(script, "\n");
    int rc = 0;
    int iflevel = 0;
    if(!labels) {
        labels = calloc(label_max, sizeof(label));
    }
    // search for labels
    for(size_t i=0; lines[i]; i++){
        lines[i] = strip(lines[i]);
        if(startswith(lines[i], "label: ")){
            // reallocate if needed
            if(label_cur >= label_max){
                label_max+= 32;
                labels = realloc(labels, sizeof(label)* label_max);
            }
            labels[label_cur].name = strdup(lines[i]+7);
            labels[label_cur].line = i;
            label_cur++;
        }
    }
    for(size_t i=0; lines[i]; i++){
        if(strlen(lines[i]) == 0){
            continue;
        }
        debug("script:%s\n", lines[i]);
        char** args = parse_line(lines[i]);
        // handle if endif
        if (iseq(args[0], "if")){
            if(operation_main(global->manager, args[1], args+2) != 0){
                iflevel++;
                int cur = i;
                // search for endif
                for(i=i+1;lines[i]; i++){
                    debug("search endif:%s %d\n", lines[i], iflevel);
                    if(strlen(lines[i]) == 0){
                        continue;
                    }
                    if(startswith(lines[i],"endif")){
                        iflevel--;
                    }
                    if(startswith(lines[i],"if ")){
                        iflevel++;
                    }
                    if(iflevel == 0){
                        goto found_endif;
                    }
                }
                error_add(build_string("syntax error at line %d : endif missing", cur));
                error(1);
                return 1;
                found_endif:
                debug("endif found at :%s %d\n", lines[i], iflevel);
            }
        }
        rc = operation_main(global->manager, args[0], args+1);
    }
    // cleanup
    for(size_t i=0; lines[i]; i++){
        free(lines[i]);
    }
    free(lines);
    return rc;
}

