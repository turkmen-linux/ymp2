#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/yaml.h>
#include <utils/fetcher.h>

#include <core/logger.h>
#include <core/variable.h>
#include <config.h>

#include <data/package.h>
#include <data/repository.h>

visible Repository* repository_new() {
    Repository* repo = (Repository*)malloc(sizeof(Repository));
    if (!repo) {
        return NULL; // Handle memory allocation failure
    }
    repo->package_count = 0; // Initialize package_count
    repo->packages = malloc(sizeof(Package*) * 32); // Initialize with a reasonable capacity
    if (!repo->packages) {
        free(repo);
        printf("Memory initial allocation failed\n");
        return NULL; // Handle memory allocation failure
    }
    return repo;
}

visible void repository_unref(Repository* repo) {
    if (!repo) return; // Check for NULL
    for (size_t i = 0; i < repo->package_count; i++) {
        package_unref(repo->packages[i]);
    }
    free(repo->packages);
    free(repo);
}

static void repository_load_data(Repository* repo, const char* data, bool is_source) {

    // Get area list
    int len = 0;
    char** areas;
    if (is_source) {
        areas = yaml_get_area_list(data, "source", &len);
    } else {
        areas = yaml_get_area_list(data, "package", &len);
    }

    if(len == 0){
        return;
    }
    debug("loaded: %d\n", len);

    // Reallocate package storage
    repo->packages = realloc(repo->packages, (repo->package_count + len) * sizeof(Package*));
    if (!repo->packages) {
        printf("Memory allocation failed %ld\n", repo->package_count + len);
        return; // Handle memory allocation failure
    }

    // Load packages
    for (int i = 0; i < len && areas[i]; i++) {
        repo->packages[repo->package_count] = package_new();
        repo->packages[repo->package_count]->is_virtual = true;
        repo->packages[repo->package_count]->repo = (void*)repo;
        if (repo->packages[repo->package_count] == NULL) {
            printf("Failed to create new package\n");
            continue; // Handle package creation failure
        }
        package_load_from_metadata(repo->packages[repo->package_count], areas[i], is_source);
        repo->package_count++;
    }
}

visible Package* repository_get(Repository *repo, const char* name, bool is_source) {
    if (repo == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < repo->package_count; i++) {
        if (repo->packages[i]->is_source != is_source) {
            continue;
        }
        if (strcmp(repo->packages[i]->name, name) == 0) {
            debug("Found package: %s\n", name);
            return repo->packages[i];
        }
    }
    debug("Not found package: %s\n", name);
    return NULL;
}

visible void repository_load_from_index(Repository* repo, const char* index) {
    debug("Load from index: %s\n", index);
    // Read index
    char* data = readfile(index);
    if (data) {
        repository_load_from_data(repo, data);
    }
}

visible void repository_load_from_data(Repository* repo, const char* data) {
    debug("Load from data len:%d\n", strlen(data));
    if (repo == NULL) {
        return;
    }
    // Get URI
    char* inner = yaml_get_area(data, "index");
    repo->name = yaml_get_value(inner, "name");
    char* repo_uri_file = build_string("%s/%s/sources.list.d/%s", get_value("DESTDIR"), STORAGE, repo->name);
    repo->uri = strip(readfile(repo_uri_file));
    // Load packages
    repository_load_data(repo, inner, true);
    repository_load_data(repo, inner, false);
}

visible bool repository_download_package(Repository* repo, const char* name, bool is_source) {
    if (repo == NULL) {
        return false;
    }
    // Get package from repository
    Package *p = repository_get(repo, name, is_source);
    if (p == NULL) {
        return false;
    }
    bool status = package_download(p, repo->uri);
    // Cleanup
    free(p);
    return status;
}

