#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>

#include <utils/file.h>

int main() {
    const char *dir_path = "example_dir/sub_dir";
    const char *file_path = "example_dir/sub_dir/example_file.txt";

    // Create a directory structure
    create_dir(dir_path);
    printf("Created directory structure: %s\n", dir_path);

    // Check if the directory exists
    if (isdir(dir_path)) {
        printf("Directory exists: %s\n", dir_path);
    } else {
        printf("Directory does not exist: %s\n", dir_path);
    }

    // Check if a file exists
    if (isexists(file_path)) {
        printf("File exists: %s\n", file_path);
    } else {
        printf("File does not exist: %s\n", file_path);
    }

    // Create a file for demonstration
    FILE *file = fopen(file_path, "w");
    if (file) {
        fprintf(file, "Hello, World!\n");
        fclose(file);
        printf("Created file: %s\n", file_path);
    } else {
        printf("Failed to create file: %s\n", file_path);
    }

    // Get the size of the file
    uint64_t size = filesize(file_path);
    printf("Size of file %s: %lu bytes\n", file_path, size);

    // Check if the file is a symbolic link
    if (issymlink(file_path)) {
        printf("File is a symbolic link: %s\n", file_path);
    } else {
        printf("File is not a symbolic link: %s\n", file_path);
    }

    // Get the directory name from the file path
    char *dir_name = dirname(file_path);
    printf("Directory name of %s: %s\n", file_path, dir_name);
    free(dir_name); // Free the allocated memory for dirname

    return 0;
}
