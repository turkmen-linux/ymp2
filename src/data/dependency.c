#include <stdlib.h>

#include <data/repository.h>
#include <core/logger.h>
#include <core/variable.h>

#include <utils/array.h>
#include <utils/file.h>
#include <utils/string.h>
#include <utils/process.h>

#include <config.h>

// Global variables for repositories, resolved dependencies, and cache
static Repository **repos;
static array *resolved;
static array *cache;
size_t depth = 0; // Variable to track the depth of dependency resolution

// Recursive function to resolve dependencies for a given package name
static void resolve_dependency_fn(char* name, bool emerge) {
    // If the package is already in the cache, return to avoid reprocessing
    if (array_has(cache, name)) {
        return;
    }
    
    // Log the current package being searched and the depth level
    debug("Search: %s depth:%d\n", name, depth);
    
    // Iterate through the repositories to find the package
    for (size_t i = 0; repos[i]; i++) {
        Package *p = repository_get(repos[i], name, emerge); // Get the package from the repository
        if (!p) {
            continue; // If the package is not found, continue to the next repository
        }
        
        depth++; // Increase the depth for the current package
        // Recursively resolve dependencies for each dependency of the package
        for (size_t j = 0; p->dependencies[j]; j++) {
            resolve_dependency_fn(p->dependencies[j], emerge);
        }
        depth--; // Decrease the depth after processing all dependencies
        
        // Log the resolved package and current depth
        debug("Resolved: %s depth:%d\n", name, depth);
        
        // Add the resolved package to the list of resolved dependencies
        array_add(resolved, name);
        i++; // Increment the repository index (this seems to be an error, should be removed)
    }
    
    // Add the package to the cache to avoid reprocessing in the future
    array_add(cache, name);
}

// Function to initialize the resolution process
static void resolve_begin() {
    resolved = array_new(); // Create a new array for resolved dependencies
    cache = array_new(); // Create a new array for caching resolved packages
    
    // Build the path to the repository index
    char* repodir = build_string("%s/%s/index", get_value("DESTDIR"), STORAGE);
    char** dirs = listdir(repodir); // List the directories in the repository
    
    size_t i = 0;
    // Count the number of repositories
    while (dirs[i]) {
        i++;
    }
    
    // Allocate memory for the repository pointers
    repos = calloc(i + 1, sizeof(Repository*));
    i = 0;
    // Load each repository from the index
    while (dirs[i]) {
        repos[i] = repository_new(); // Create a new repository instance
        repository_load_from_index(repos[i], dirs[i]); // Load the repository data
        i++; // Increment the directory index
    }
    
    // Free the directory list and the repository directory string
    free(dirs);
    free(repodir);
}

// Function to clean up resources after dependency resolution
static void resolve_end() {
    // Unreference and free each repository
    for (size_t i = 0; repos[i]; i++) {
        repository_unref(repos[i]);
    }
    free(repos); // Free the repository pointer array
    array_unref(resolved); // Unreference the resolved dependencies array
    array_unref(cache); // Unreference the cache array
}

// Public function to resolve dependencies for a given package name
visible char** resolve_dependency(char* name) {
    size_t begin_time = get_epoch();
    resolve_begin(); // Initialize the resolution process
    resolve_dependency_fn(name, iseq(get_value("no-emerge"), "")); // Resolve dependencies recursively
    size_t len;
    char** ret = array_get(resolved, &len); // Get the resolved dependencies as an array
    resolve_end(); // Clean up resources
    info("Dependencies resolved in %s µs\n", get_epoch() - begin_time);
    return ret; // Return the array of resolved dependencies
}

