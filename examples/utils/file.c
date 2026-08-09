#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
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

    // List directory
    char **dirs = listdir("/");
    size_t i = 0;
    while (dirs[i]) {
        // dont print hidden
        if (dirs[i][0] != '.') {
            printf("%s\n", dirs[i]);
        }
        i++;
    }
    // Find (list files recursivelly)
    char **files = find("./");
    i = 0;
    while (files[i]) {
        printf("%s\n", files[i]);
        i++;
    }
    // command output
    char *args[] = { "/bin/cat", "/etc/os-release", NULL };
    char *out = getoutput(args);
    printf("%s\n", out);

    const char *source = file_path;
    const char *destination = "destination.txt";

    if (copy_file(source, destination)) {
        printf("File copied successfully from %s to %s\n", source, destination);
    } else {
        printf("Failed to copy file from %s to %s\n", source, destination);
    }

    if (copy_directory("example_dir", "example_dir2")) {
        printf("File copied successfully from %s to %s\n", source, destination);
    } else {
        printf("Failed to copy file from %s to %s\n", source, destination);
    }

    // remove files
    create_dir("dir1/23");
    create_dir("dir1/45");
    writefile("dir1/23/test.txt", "Test123");
    writefile("dir1/45/test2.txt", "Test123");
    if (!remove_all("dir1")) {
        return 1;
    }

    return 0;
}
