#include <stdio.h>
#include <stdlib.h>

#include <core/ymp.h>
#include <core/logger.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>

#include <data/build.h>

visible char* ympbuild_get_value(ympbuild* ymp, const char* name) {
    char* command = build_string("exec 2>/dev/null ; %s \necho ${%s}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput(args));
    debug("variable: %s -> %s\n", name, output);
    return output;
}

visible char** ympbuild_get_array(ympbuild* ymp, const char* name){
    char* command = build_string("exec 2>/dev/null ; %s \necho ${%s[@]}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput(args));
    debug("variable: %s -> %s\n", name, output);
    return split(output," ");

}

visible bool build_from_path(const char* path){
    if(!isfile(build_string("%s/ympbuild",path))){
        return false;
    }
    // configure header
    ympbuild *ymp = malloc(sizeof(ympbuild));
    ymp->ctx = readfile(build_string("%s/ympbuild",path));
    ymp->header = readfile(":/ympbuild-header.sh");
    // Create build path
    ymp->path = calculate_md5(build_string("%s/ympbuild",path));
    ymp->path = build_string("/tmp/ymp-build/%s", ymp->path);
    ymp->header = str_replace(ymp->header, "@buildpath@", ymp->path);
    create_dir(ymp->path);
    // fetch values
    char* name = ympbuild_get_value(ymp, "name");
    char** deps = ympbuild_get_array(ymp, "depends");
    (void)name; (void)deps;
    return true;
}
