#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <core/ymp.h>
#include <core/variable.h>

#include <data/quarantine.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/hash.h>
#include <utils/jobs.h>

// Function to validate files in the quarantine directory
static int quarantine_validate_files(const char* name) {
    printf("Validate %s (files)\n", name);
    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    
    // Initialize status to indicate success or failure
    int status = 0;
    
    // Build the path to the files in quarantine
    char* files_path = build_string("%s/%s/quarantine/files/%s", destdir, STORAGE, name);
    
    // Check if the files_path is a valid file
    if (!isfile(files_path)) {
        return 0;
    }
    
    // Open the files for reading
    FILE *files = fopen(files_path, "r");
    char sha1[40]; // Buffer to hold SHA1 hash
    char line[1024 + 41]; // Buffer for reading lines (max file name length is 1024)

    // Read each line from the files
    while (fgets(line, sizeof(line), files)) {
        // Trim newline characters from the end of the line
        while (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        
        // Check if the line is valid (length should be greater than 40)
        if (strlen(line) <= 40) {
            status = 1;
            goto free_quarantine_package;
        }
        
        // Extract SHA1 hash and file path from the line
        strncpy(sha1, line, 40);
        
        // Build the actual file path in quarantine root filesystem
        char* actual_file = build_string("%s/%s/quarantine/rootfs/%s", destdir, STORAGE, line + 41);
        
        // Check if the actual file exists
        if (!isfile(actual_file)) {
            status = 1;
        } else {
            // Calculate the SHA1 hash of the actual file
            char* actual_sha1 = calculate_sha1(actual_file);
            // Compare the calculated hash with the expected hash
            status = strncmp(actual_sha1, sha1, 40);
            free(actual_sha1);
        }
        
        free(actual_file);
        
        // If any status indicates failure, jump to cleanup
        if (status) {
            goto free_quarantine_package;
        }
    }

free_quarantine_package:
    // Cleanup: free allocated memory and close the file
    free(files_path);
    fclose(files);
    return status;
}

// Function to validate all quarantine metadata files
visible bool quarantine_validate() {
    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    
    // Build the path to the metadata directory
    char* metadata = build_string("%s/%s/quarantine/metadata", destdir, STORAGE);
    
    // Find all metadata files in the directory
    char** metadatas = find(metadata);
    
    // Create a new job queue
    jobs* j = jobs_new();
    
    // Iterate through each metadata file
    for (size_t i = 0; metadatas[i]; i++) {
        // Check if the file has a .yaml extension
        if (endswith(metadatas[i], ".yaml")) {
            // Remove the .yaml extension for processing
            metadatas[i][strlen(metadatas[i]) - 5] = '\0';
            // Add a job to validate the corresponding files
            jobs_add(j, (callback)quarantine_validate_files, basename(metadatas[i]), NULL);
        }
    }
    
    // Run the jobs and check for failures
    jobs_run(j);
    bool status = j->failed; // Capture the failure status
    jobs_unref(j); // Unreference the job queue

    // Cleanup: free allocated memory for metadata file names
    for (size_t i = 0; metadatas[i]; i++) {
        free(metadatas[i]);
    }
    free(metadata);
    free(metadatas);
    return status;
}

