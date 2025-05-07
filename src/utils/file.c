#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

visible uint64_t filesize(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return 0; // Return 0 if the file does not exist or an error occurs
}

visible bool issymlink(const char *filename) {
    if (filename == NULL) {
        return false;
    }
    struct stat st;
    return (lstat(filename, &st) == 0) && S_ISLNK(st.st_mode);
}

visible bool isexists(const char* path) {
    struct stat path_stat;
    return (stat(path, &path_stat) == 0);
}

visible bool isdir(const char *path) {
    if (path == NULL || issymlink(path)) {
        return false;
    }
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return true;
    }
    return false;
}

visible char* dirname(const char* path) {
    // Check if the path is NULL or empty
    if (path == NULL || *path == '\0') {
        return strdup(""); // Return an empty string
    }

    // Find the length of the path
    size_t path_length = strlen(path);
    
    // Create a buffer to hold the directory name
    char* ret = (char*)malloc(path_length + 1); // Allocate enough space
    if (ret == NULL) {
        return NULL; // Handle memory allocation failure
    }
    
    size_t ret_index = 0;

    // Check if the path starts with '/'
    if (path[0] == '/') {
        ret[ret_index++] = '/'; // Start with '/' if the path is absolute
    }

    // Split the path by '/' and build the directory name
    for (size_t i = 1; i < path_length; i++) {
        if (path[i] == '/') {
            // If we encounter a '/', we add it to the result
            if (ret_index > 0 && ret[ret_index - 1] != '/') {
                ret[ret_index++] = '/';
            }
        } else {
            // Add the character to the result
            ret[ret_index++] = path[i];
        }
    }

    // Remove the last character if it's a '/'
    if (ret_index > 1 && ret[ret_index - 1] == '/') {
        ret_index--;
    }

    // Null-terminate the string
    ret[ret_index] = '\0';

    return ret; // Return the newly allocated directory string
}


visible void create_dir(const char *dir) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    
    // Remove trailing slash if present
    if (len > 0 && tmp[len - 1] == '/') {
        tmp[len - 1] = '\0';
    }

    // Create directories in the path
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0'; // Temporarily terminate the string
            mkdir(tmp, 0755); // Create the directory
            *p = '/'; // Restore the string
        }
    }
    mkdir(tmp, 0755); // Create the final directory
}

