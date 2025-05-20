#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/yaml.h>

#include <data/repository.h>

visible Repository* repository_new(){
    return (Repository*)malloc(sizeof(Repository));
}

visible void repository_load_from_index(Repository* repo, const char* index){
    // read index
    char* data = readfile(index);
    // get uri
    char* inner = yaml_get_area(data, "index");
    repo->uri = yaml_get_value(inner, "address");
    puts(repo->uri);
    free(data);
    // get area list
    int len = 0;
    char** areas = yaml_get_area_list(inner, "source", &len);
    // print
    for(int i=0; i<len; i++){
        printf(areas[i]);
    }
}