#define _GNU_SOURCE
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
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <libgen.h>
#include <sched.h>
#include <errno.h>

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

visible bool isfile(const char *filename) {
    debug("check is file: %s\n", filename);
    if (filename == NULL) {
        return false;
    }
    struct stat st;
    return (stat(filename, &st) == 0) && S_ISREG(st.st_mode);
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
            free(inodes[i]);
            i++;
            continue;
        }
        char* inode = build_string("%s/%s", path, inodes[i]);
        if(isdir(inode)){
            find_operation(array, inode);
        } else {
            array_add(array, inode);
        }
        free(inode);
        free(inodes[i]);
        i++;
    }
    free(inodes);
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

visible void writefile(const char* path, const char* data) {
    debug("write to file: %s\n", path);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    size_t length = strlen(data);
    size_t written = fwrite(data, sizeof(char), length, file);
    if (written != length) {
        perror("Error writing to file");
    }

    fclose(file);
}
visible char* getoutput_unshare(char* argv[], int flags) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return NULL;
    }
    if (pid == 0) { // Child process
        // Close the read end of the pipe
        close(pipefd[0]);

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Close the original write end

        // unshare flags
        if(unshare(flags) < 0){
            exit(EXIT_FAILURE);
        }
        // Execute the command
        execvp(argv[0], argv);

        // If execvp returns, it must have failed
        perror("execvp");
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Close the write end of the pipe
        close(pipefd[1]);

        // Read the output from the read end of the pipe
        char* ret = NULL;
        size_t bufsize = 1024;
        ret = calloc(bufsize, sizeof(char));
        if (!ret) {
            perror("Memory allocation error");
            return "";
        }

        size_t total_read = 0;
        char buff[1024];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buff, sizeof(buff) - 1)) > 0) {
            buff[bytes_read] = '\0'; // Null-terminate the buffer
            size_t len = total_read + bytes_read + 1;
            if (len > bufsize) {
                // Resize buffer if needed
                bufsize = len * 2;
                ret = realloc(ret, bufsize);
                if (!ret) {
                    perror("Memory reallocation error");
                    return "";
                }
            }
            strcat(ret, buff);
            total_read += bytes_read;
        }

        close(pipefd[0]); // Close the read end of the pipe

        // Wait for the child process to finish
        int status;
        wait(&status);

        // Trim the buffer to the actual size needed
        ret = realloc(ret, (strlen(ret) + 1) * sizeof(char));

        return ret;
    }
}

visible bool copy_file(const char *sourceFile, const char *destFile) {
    debug("Copy file: %s -> %s", sourceFile, destFile);
    int source, dest;
    char buffer[1024 * 1024]; // Buffer to hold data (1MB)
    ssize_t bytesRead;

    struct stat statbuf;

    // Check if the source file is a symlink
    if (lstat(sourceFile, &statbuf) < 0) {
        perror("Error getting file status");
        return false;
    }

    // If it's a symlink, read the target
    if (S_ISLNK(statbuf.st_mode)) {
        char target[PATH_MAX];
        ssize_t len = readlink(sourceFile, target, sizeof(target) - 1);
        if (len < 0) {
            perror("Error reading symlink");
            return false;
        }
        target[len] = '\0';
        // Create a symbolic link
        if (symlink(target, destFile) == -1) {
            perror("Error creating symlink");
            return false;
        }
        return true;
    }

    // Open the source file in read-only mode
    source = open(sourceFile, O_RDONLY);
    if (source < 0) {
        perror("Error opening source file");
        return false;
    }

    // Create destination file directory
    char *dir = strdup(destFile);
    dirname(dir);
    create_dir(dir);
    free(dir);

    // Open the destination file in write-only mode, create it if it doesn't exist
    dest = open(destFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest < 0) {
        perror("Error opening destination file");
        close(source);
        return false;
    }

    // Copy the file content
    while ((bytesRead = read(source, buffer, sizeof(buffer))) > 0) {
        ssize_t written = write(dest, buffer, bytesRead);
        if (written < 0) {
            perror("Error writing file");
            close(source);
            close(dest);
            return false;
        }
    }

    // Close the files
    close(source);
    close(dest);
    return true;
}


// Function to copy a directory recursively
visible bool copy_directory(const char *sourceDir, const char *destDir) {
    struct stat st;
    if (stat(sourceDir, &st) != 0) {
        perror("Error accessing source directory");
        printf("%s\n", sourceDir);
        return false;
    }

    // Create the destination directory
    if (!isdir(destDir) && mkdir(destDir, st.st_mode) != 0) {
        perror("Error creating destination directory");
        printf("%s\n", destDir);
        return false;
    }

    DIR *dir = opendir(sourceDir);
    if (dir == NULL) {
        perror("Error opening source directory");
        printf("%s\n", sourceDir);
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip the "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char sourcePath[1024];
        char destPath[1024];

        snprintf(sourcePath, sizeof(sourcePath), "%s/%s", sourceDir, entry->d_name);
        snprintf(destPath, sizeof(destPath), "%s/%s", destDir, entry->d_name);

        if (entry->d_type == DT_DIR) {
            // Recursively copy the directory
            if (!copy_directory(sourcePath, destPath)) {
                closedir(dir);
                return false;
            }
        } else {
            // Copy the file
            if (!copy_file(sourcePath, destPath)) {
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);
    return true;
}

visible bool move_file(const char* src, const char* dest){
    int status = rename(src, dest);
    if(status < 0){
        if(copy_file(src, dest)){
            unlink(src);
        } else {
            perror("Failed to move file!");
            return false;
        }
    }
    return true;
}

visible char* sreadlink(const char* path) {
    // Buffer size for the target path
    ssize_t bufsize = 1024; // You can adjust this size as needed
    char* buf = malloc(bufsize);
    if (buf == NULL) {
        perror("malloc");
        return NULL; // Return NULL if memory allocation fails
    }

    ssize_t len = readlink(path, buf, bufsize - 1);
    if (len == -1) {
        perror("readlink");
        free(buf); // Free the allocated memory on error
        return NULL; // Return NULL on error
    }

    buf[len] = '\0'; // Null-terminate the string
    return buf; // Return the dynamically allocated string
}

visible	bool remove_all(const char *path) {
    // Struct for path information
    struct stat st;
    // Check access
    if (stat(path, &st) != 0) {
        fprintf(stderr, "Cannot access: %s\n", path);
        return false;
    }

    // Check is directory
    if (S_ISDIR(st.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) {
            perror(path); // Failed to open directory
            return false;
        }

        // Look for sub directories
        char buf[4096];
        struct dirent *e;
        while ((e = readdir(dir))) {
            // Skip ./ and ../
            if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) {
                continue;
            }

            // Remove sub directories
            snprintf(buf, sizeof(buf), "%s/%s", path, e->d_name);
            if (!remove_all(buf)) {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);

        // Remove empty directory
        if (rmdir(path) != 0) {
            perror(path); // remove error
            return false;
        }
    } else {
        // Remove file
        if (unlink(path) != 0) {
            perror(path); // remove error
            return false;
        }
    }
    return true;

}
