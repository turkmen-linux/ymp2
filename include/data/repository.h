#ifndef _repository_h
#define _repository_h
#include <data/package.h>

typedef struct {
    const char* uri;
    Package** packages;
    size_t package_count;
} Repository;


Repository* repository_new();
void repository_load_from_index(Repository* repo, const char* path);
void repository_load_from_data(Repository* repo, const char* data);
bool repository_download_package(Repository* repo, const char* name, bool is_source);
#endif
