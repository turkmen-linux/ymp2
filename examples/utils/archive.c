#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <core/ymp.h>
#include <utils/archive.h>

int main() {

    (void)ymp_init();
    // Write some content to the file
    FILE *file = fopen("file1.txt", "w");
    if(!file){
        return 1;
    }
    fprintf(file, "This is the content of file1.txt.\n");
    fprintf(file, "It contains some sample text for the archive example.\n");
    fclose(file);

    // Write some content to the file
    file = fopen("file2.txt", "w");
    if(!file){
        return 1;
    }
    fprintf(file, "This is the content of file2.txt.\n");
    fprintf(file, "It contains some sample text for the archive example.\n");
    fclose(file);

    // Write some content to the file
    file = fopen("file3.txt", "w");
    if(!file){
        return 1;
    }
    fprintf(file, "This is the content of file3.txt.\n");
    fprintf(file, "It contains some sample text for the archive example.\n");
    fclose(file);

    // Create a new Archive instance
    Archive *myArchive = archive_new();
    if (!myArchive) {
        fprintf(stderr, "Failed to create archive instance.\n");
        return EXIT_FAILURE;
    }

    // Load the archive from the specified path
    archive_load(myArchive, "my_archive.zip");

    // Set the type and filter for the archive (e.g., creating a zip archive with no filter)
    archive_set_type(myArchive, "zip", "none");

    // Add files to the archive
    archive_add(myArchive, "file1.txt");
    archive_add(myArchive, "file2.txt");
    archive_add(myArchive, "file3.txt");

    // Create the archive
    archive_create(myArchive);

    free(myArchive);

    myArchive = archive_new();


    // Load the archive from the specified path
    archive_load(myArchive, "my_archive.zip");

    mkdir("./extracted_files", 0755);
    // Set the target extraction path
    archive_set_target(myArchive, "./extracted_files");


    // Check if the path is a valid archive
    if (archive_is_archive(myArchive, "my_archive.zip")) {
        printf("The path is a valid archive.\n");

        // List files in the archive
        size_t fileCount;
        char **files = archive_list_files(myArchive, &fileCount);
        printf("Files in the archive:\n");
        for (size_t i = 0; i < fileCount; i++) {
            printf(" - %s\n", files[i]);
            free(files[i]); // Free each file name after use
        }
        free(files); // Free the array of file names

        // Extract all files from the archive
        archive_extract_all(myArchive);
    } else {
        printf("The path is not a valid archive.\n");
    }

    // Clean up and release resources
    free(myArchive);

    return EXIT_SUCCESS;
}
