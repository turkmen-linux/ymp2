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



visible char** get_group_packages (const char* name) {
    info("reslove group: %s depth:%d\n", name, depth);
    array *res = array_new();
    Package *pi = package_new();
    pi->is_virtual = true;
    for (size_t i = 0; repos[i]; i++) {
       for(size_t j=0; j< repos[i]->package_count;j++){
            if(strcmp(name+1, "universe") == 0){
               array_add(res, repos[i]->packages[j]->name);
            }else if(strcmp(name+1, "world") == 0){
                pi->name = repos[i]->packages[j]->name;
                if(package_is_installed(pi)){
                    array_add(res, repos[i]->packages[j]->name);
                }
            } else {
                for(size_t g=0; repos[i]->packages[j]->groups[g];g++){
                    const char* grp = repos[i]->packages[j]->groups[g];
                    size_t l1 = strlen(name)-1;
                    size_t l2 = strlen(grp);
                    if(l2 < l1){
                        // len(grp) < len(name)
                        continue;
                    } else if(l1 == l2 && strncmp(grp, name+1, l1) == 0){
                        // len(grp) == len(name)
                        array_add(res, repos[i]->packages[j]->name);
                    } else if(strncmp(grp, name+1, l2) == 0 && grp[l1] == '.'){
                        // len(grp) > len(name) and grp[len(name)] == '.'
                        array_add(res, repos[i]->packages[j]->name);
                    }
                }
            }
       }
    }
    size_t len;
    char** ret=array_get(res, &len);
    free(res);
    return ret;
}

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

    if(name[0] == '@'){
        char** grp_pkgs = get_group_packages(name);
        for(size_t i=0; grp_pkgs[i]; i++){
            resolve_dependency_fn(grp_pkgs[i], emerge);
        }
        return;
    }

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

// Recursive function to resolve dependencies for a given package name
static void resolve_reverse_dependency_fn(char* name) {
    // If the package is already in the cache, return to avoid reprocessing
    if (array_has(cache, name)) {
        return;
    }

    // Add the package to the cache to avoid reprocessing in the future
    array_add(cache, name);

    // Log the current package being searched and the depth level
    info("Search: %s depth:%d\n", name, depth);


    // Check list length reallocate if needed
    if(resolved_count+1 <= resolved_total) {
        resolved_total += 1024;
        resolved = realloc(resolved, sizeof(Package*)*resolved_total);
    }
    Package* pkg = package_new();
    if(!package_load_from_installed(pkg, name)){
        return;
    }
    // Add the resolved package to the list of resolved packages
    resolved[resolved_count] = pkg;
    resolved_count++;

    // load installed package object
    char* metadata_dir = build_string("%s/%s/metadata", get_value("DESTDIR"), STORAGE);
    char** packages = listdir(metadata_dir);
    depth++; // Increase the depth for the current package
    for(size_t i=0; packages[i]; i++){
        if(!endswith(packages[i], ".yaml")){
            continue;
        }
        Package* pi = package_new();
        packages[i][strlen(packages[i])-5] = '\0';
        package_load_from_installed(pi, packages[i]);
        for(size_t j = 0; pi->dependencies[j];j++){
            if(iseq(name, pi->dependencies[j])){
                resolve_reverse_dependency_fn(packages[i]);
            }
        }
    }
    depth--; // Decrease the depth after processing all dependencies

    // Log the resolved package and current depth
    info("Resolved: %s depth:%d\n", name, depth);

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


// Public function to resolve reverse dependencies for a given package name
visible Package** resolve_reverse_dependency(char* name) {
    size_t begin_time = get_epoch();
    if(resolved != NULL){
        free(resolved);
    }
    resolved = malloc(sizeof(Package*)* 1024); // Create a new array for resolved packages
    resolved_count = 0; // reset resolve count
    resolved_total = 0; // reset resolve total
    cache = array_new(); // Create a new array for caching resolved packages
    info("Reverse dependencies resolved in %s µs\n", get_epoch() - begin_time);
    resolve_reverse_dependency_fn(name);
    resolved[resolved_count] = NULL;
    return resolved; // Return the array of resolved dependencies
}