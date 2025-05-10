#include <core/ymp.h>
#include <utils/string.h>
#include <utils/file.h>

visible bool build_from_path(const char* path){
    char* header = readfile(":/ympbuild-header.sh");
    if(!isfile(build_string("%s/ympbuild",path))){
        return false;
    }
    header = str_replace(header, "@buildpath@", path);
    return true;
}
