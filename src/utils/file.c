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

#include <utils/array.h>
#include <utils/string.h>
#include <core/logger.h>

visible uint64_t filesize(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        debug("file size: %s %lld\n", path, st.st_size);
        return st.st_size;
    }
    return 0; // Return 0 if the file does not exist or an error occurs
}

visible bool issymlink(const char *filename) {
    debug("check is symlink: %s\n", filename);
    if (filename == NULL) {
        return false;
    }
    struct stat st;
    return (lstat(filename, &st) == 0) && S_ISLNK(st.st_mode);
}

visible bool isexists(const char* path) {
    debug("check is exists: %s\n", path);
    struct stat path_stat;
    return (stat(path, &path_stat) == 0);
}

visible bool isdir(const char *path) {
    debug("check is directory: %s\n", path);
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

visible void create_dir(const char *dir) {
    debug("create directory: %s\n", dir);
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

visible char** listdir(const char* path){
    debug("list directory: %s\n", path);
    DIR *dp;
    struct dirent *ep;
    dp = opendir (path);
    array *a = array_new();
    if (dp != NULL) {
        while ((ep = readdir (dp))) {
            array_add(a,ep->d_name);
        }
        (void) closedir (dp);
    }
    size_t len = 0;
    char** dirs = array_get(a, &len);
    array_unref(a);
    return dirs;
}

static void find_operation(array* array, const char* path){
    debug("find files from: %s\n", path);
    char** inodes = listdir(path);
    int i=0;
    while(inodes[i]){
        if(iseq(inodes[i], "..") || iseq(inodes[i], ".")){
            i++;
            continue;
        }
        char* inode = build_string("%s/%s", path, inodes[i]);
        if(isdir(inode)){
            find_operation(array, inode);
        }
        array_add(array, inode);
        i++;
    }
}

visible char** find(const char* path){
    debug("find files: %s\n", path);
    array* a = array_new();
    find_operation(a, path);
    size_t len;
    char** list = array_get(a, &len);
    array_unref(a);
    return list;
}
