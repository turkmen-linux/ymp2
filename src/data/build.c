#include <core/ymp.h>
#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>
#include <stdlib.h>

typedef struct {
    char* ctx;
    char* path;
    char* header;
} ympbuild;

static char* ympbuild_get_value(ympbuild* ymp, const char* name) {
    char* command = build_string("%s; echo $%s", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    return getoutput(args);
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
    (void)name;
    return true;
}
