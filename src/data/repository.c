#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/yaml.h>
#include <utils/fetcher.h>

#include <core/variable.h>
#include <config.h>

#include <data/package.h>
#include <data/repository.h>

visible Repository* repository_new(){
    Repository* repo = (Repository*)malloc(sizeof(Repository));
    repo->packages = malloc(sizeof(Package*));
    return repo;
}

visible void repository_unref(Repository* repo){
    for(size_t i=0; i<repo->package_count; i++){
        package_unref(repo->packages[i]);
    }
    free(repo->packages);
    free(repo);
}

static void repository_load_data(Repository* repo, const char* data, bool is_source){
    // get area list
    int len = 0;
    char** areas;
    if(is_source) {
        areas = yaml_get_area_list(data, "source", &len);
    } else {
        areas = yaml_get_area_list(data, "package", &len);
    }
    repo->packages = realloc(repo->packages,
        (repo->package_count + (size_t)len + 1)* sizeof(Package));
    // print
    for(int i=0; i<len; i++){
        repo->packages[repo->package_count] = package_new();
        package_load_from_metadata(repo->packages[repo->package_count], areas[i], is_source);
        repo->package_count++;
    }

}

visible Package* repository_get(Repository *repo, const char* name, bool is_source){
    if(repo == NULL){
        return NULL;
    }
    for(size_t i=0; i<repo->package_count;i++){
        if (repo->packages[i]->is_source != is_source){
            continue;
        }
        if (strcmp(repo->packages[i]->name, name) == 0){
            return repo->packages[i];
        }
    }
    return NULL;
}

visible void repository_load_from_index(Repository* repo, const char* index){
    // read index
    char* data = readfile(index);
    repository_load_from_data(repo, data);
    // cleanup
    free(data);
}
visible void repository_load_from_data(Repository* repo, const char* data){
    if(repo == NULL){
        return;
    }
    // get uri
    char* inner = yaml_get_area(data, "index");
    repo->uri = yaml_get_value(inner, "address");
    // load packages
    repository_load_data(repo, inner, true);
    repository_load_data(repo, inner, false);

}

visible bool repository_download_package(Repository* repo, const char* name, bool is_source){
    if(repo == NULL){
        return false;
    }
    // get package from repository
    Package *p = repository_get(repo, name, is_source);
    if(p == NULL){
        return false;
    }
    // generate download uri
    char* uri = str_replace(repo->uri, "$uri", yaml_get_value(p->metadata, "uri"));
    // download file into cache
    char* destdir = get_value("DESTDIR");
    char* pkgname = build_string("%s-%s-%d", p->name, p->version, p->release);
    char* target = build_string("%s/%s/packages/%s", destdir, STORAGE, basename(uri));
    // fetch package
    bool status = fetch(uri, target);
    // cleanup
    free(pkgname);
    free(target);
    free(uri);
    // return status
    return status;
}
