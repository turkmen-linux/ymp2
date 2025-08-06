#include <stdlib.h>
#include <stdio.h>

#include <data/repository.h>
#include <core/logger.h>
#include <core/variable.h>

#include <utils/array.h>
#include <utils/file.h>
#include <utils/string.h>
#include <utils/process.h>

#include <config.h>

static Package **resolved;
static size_t resolved_count = 0;
static size_t resolved_total = 0;

// Global variables for repositories, resolved dependencies, and cache
static Repository** repos;
static array *cache;
size_t depth = 0; // Variable to track the depth of dependency resolution

// Recursive function to resolve dependencies for a given package name
static void resolve_dependency_fn(char* name, bool emerge) {
    // If the package is already in the cache, return to avoid reprocessing
    if (array_has(cache, name)) {
        return;
    }

    // Add the package to the cache to avoid reprocessing in the future
    array_add(cache, name);

    // Log the current package being searched and the depth level
    info("Search: %s depth:%d\n", name, depth);

    // Iterate through the repositories to find the package
    for (size_t i = 0; repos[i]; i++) {
        Package *p = repository_get(repos[i], name, emerge); // Get the package from the repository
        if (!p || !p->dependencies) {
            continue; // If the package is not found, continue to the next repository
        }

        depth++; // Increase the depth for the current package
        // Recursively resolve dependencies for each dependency of the package
        for (size_t j = 0; p->dependencies[j]; j++) {
            resolve_dependency_fn(p->dependencies[j], emerge);
        }
        depth--; // Decrease the depth after processing all dependencies

        // Log the resolved package and current depth
        info("Resolved: %s depth:%d\n", name, depth);


        // Check list length reallocate if needed
        if(resolved_count+1 <= resolved_total) {
            resolved_total += 1024;
            resolved = realloc(resolved, sizeof(Package*)*resolved_total);
        }
        // Add the resolved package to the list of resolved packages
        resolved[resolved_count] = p;
        resolved_count++;
    }

}

// Function to initialize the resolution process
visible Repository** resolve_begin() {


    // Build the path to the repository index
    char* repodir = build_string("%s/%s/index", get_value("DESTDIR"), STORAGE);
    char** dirs = listdir(repodir); // List the directories in the repository

    size_t i = 0;
    size_t j = 0;
    // Count the number of repositories
    while (dirs[i]) {
        if(!endswith(dirs[i], ".yaml")){
            i++;
            continue;
        }
        j++;
        i++;
    }

    // Allocate memory for the repository pointers
    repos = calloc(j, sizeof(Repository*));
    i = 0;
    j=0;
    // Load each repository from the index
    while (dirs[i]) {
        if(!endswith(dirs[i], ".yaml")){
            i++;
            continue;
        }
        char* index = build_string("%s/%s", repodir, dirs[i]);
        repos[j] = repository_new(); // Create a new repository instance
        repository_load_from_index(repos[j], index); // Load the repository data
        i++;
        j++;
        free(index);
    }

    // Free the directory list and the repository directory string
    free(dirs);
    free(repodir);
    return repos;
}

// Function to clean up resources after dependency resolution
visible void resolve_end(Repository** repos) {
    // Unreference and free each repository
    for (size_t i = 0; repos[i]; i++) {
        repository_unref(repos[i]);
    }
    free(repos); // Free the repository pointer array
    free(resolved); // Unreference the resolved dependencies array
    array_unref(cache); // Unreference the cache array
}

// Public function to resolve dependencies for a given package name
visible Package** resolve_dependency(char* name) {
    size_t begin_time = get_epoch();
    if(resolved != NULL){
        free(resolved);
    }
    if(repos == NULL){
        print("Dependencies resolve failed\n");
        return NULL; // Dont resolve package if repository list is empty
    }
    resolved = malloc(sizeof(Package*)* 1024); // Create a new array for resolved packages
    resolved_count = 0; // reset resolve count
    resolved_total = 0; // reset resolve total
    cache = array_new(); // Create a new array for caching resolved packages
    
    resolve_dependency_fn(name, iseq(get_value("no-emerge"), "")); // Resolve dependencies recursively
    info("Dependencies resolved in %s µs\n", get_epoch() - begin_time);
    return resolved; // Return the array of resolved dependencies
}

