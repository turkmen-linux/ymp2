#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>

#include <config.h>
#include <core/ymp.h>
#include <core/variable.h>
#include <core/logger.h>

#include <data/quarantine.h>

#include <utils/file.h>
#include <utils/string.h>
#include <utils/hash.h>
#include <utils/jobs.h>

// Function to validate files in the quarantine directory
static int quarantine_validate_files(const char* name) {
    print("%s: %s (%s)\n", colorize(YELLOW, "Validate"), name, "files");
    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");

    // Initialize status to indicate success or failure
    int status = 0;

    // Build the path to the files in quarantine
    char* files_path = build_string("%s/%s/quarantine/files/%s", destdir, STORAGE, name);
    char* rootfs_path = build_string("%s/%s/quarantine/rootfs/", destdir, STORAGE);

    // Check if the files_path is a valid file
    if (!isfile(files_path)) {
        free(files_path);
        free(rootfs_path);
        return 0;
    }

    // Open the files for reading
    FILE *files = fopen(files_path, "r");
    char line[PATH_MAX + 41]; // Buffer for reading lines (max file name length is PATH_MAX)
    char actual_file[PATH_MAX +	strlen(rootfs_path)]; // Buffer for actual file path (max file name length is PATH_MAX)

    // Read each line from the files
    while (fgets(line, sizeof(line), files)) {
        // Trim newline characters from the end of the line
        while (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }

        // Check if the line is valid (length should be greater than 40)
        if (strlen(line) <= 40) {
            status = 1;
            goto free_quarantine_validate_files;
        }

        // Build the actual file path in quarantine root filesystem
        strcpy(actual_file, rootfs_path);
        strcat(actual_file, line + 41);

        debug("Validate file: %s\n", actual_file+strlen(rootfs_path));
        // Check if the actual file exists
        if (!isfile(actual_file)) {
            status = 1;
        } else {
            // Calculate the SHA1 hash of the actual file
            char* actual_sha1 = calculate_sha1(actual_file);
            // Compare the calculated hash with the expected hash
            status = strncmp(actual_sha1, line, 40);
            free(actual_sha1);
        }

        // If any status indicates failure, jump to cleanup
        if (status) {
            goto free_quarantine_validate_files;
        }
    }

free_quarantine_validate_files:
    // Cleanup: free allocated memory and close the file
    free(files_path);
    free(rootfs_path);
    fclose(files);
    return status;
}

static int quarantine_validate_links(const char* name){
    print("%s: %s (%s)\n", colorize(YELLOW, "Validate"), name, "links");
    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");

    // Initialize status to indicate success or failure
    int status = 0;

    // Build the path to the files in quarantine
    char* links_path = build_string("%s/%s/quarantine/links/%s", destdir, STORAGE, name);
    char* rootfs_path = build_string("%s/%s/quarantine/rootfs/", destdir, STORAGE);

    // Check if the files_path is a valid file
    if (!isfile(links_path)) {
        free(links_path);
        free(rootfs_path);
        return 0;
    }

    // Open the files for reading
    FILE *links = fopen(links_path, "r");
    char line[PATH_MAX]; // Buffer for reading lines (max file name length is PATH_MAX)
    char actual_link[PATH_MAX + strlen(rootfs_path)]; // Buffer for actual file path (max file name length is PATH_MAX)

    // Read each line from the files
    while (fgets(line, sizeof(line), links)) {
        // Trim newline characters from the end of the line
        while (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        // calcuate offset of link - path seperator
        size_t offset=0;
        while(line[offset] && line[offset] != ' ') {
            offset++;
        }
        // Build actual_link
        strcpy(actual_link, rootfs_path);
        strcat(actual_link, line+offset+1);
        // Build link target
        line[offset+1]='\0';
        ssize_t rc = readlink(actual_link, line+offset+2, PATH_MAX-(offset+2));
        if(rc <0){
            perror("Error reading symlink");
            status = 1;
            goto free_quarantine_validate_links;
        }
        line[offset+rc+2] = '\0';
        debug("Validate link: %s => %s\n", line, actual_link+strlen(rootfs_path));
        // check links are same
        status = strncmp(line, line+offset+2, offset);
        if (status){
            goto free_quarantine_validate_links;
        }
        // check link is absolute path
        if(line[0] == '/'){
            status = 1;
            goto free_quarantine_validate_links;
        }
    }
free_quarantine_validate_links:
    // Cleanup: free allocated memory and close the file
    free(links_path);
    free(rootfs_path);
    fclose(links);
    return status;
}

// Function to sync quarantine validated files
visible int quarantine_sync(const char* name){
    print("%s: %s \n", colorize(YELLOW, "Sync"), name);
    int status = 0;
    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    char* rootfs_path = build_string("%s/%s/quarantine/rootfs/", destdir, STORAGE);

    // Build the path to the files and links in quarantine
    char* metadata_path = build_string("%s/%s/quarantine/metadata/%s.yaml", destdir, STORAGE, name);
    char* files_path = build_string("%s/%s/quarantine/files/%s", destdir, STORAGE, name);
    char* links_path = build_string("%s/%s/quarantine/links/%s", destdir, STORAGE, name);

    // Open the files for reading
    FILE *links = fopen(links_path, "r");
    FILE *files = fopen(files_path, "r");
    char line[PATH_MAX]; // Buffer for reading lines (max file name length is PATH_MAX)
    char source[PATH_MAX + strlen(rootfs_path)]; // Buffer for source path (max file name length is PATH_MAX)
    char target[PATH_MAX + strlen(rootfs_path)]; // Buffer for target path (max file name length is PATH_MAX)


    char tmp[PATH_MAX + strlen(rootfs_path)]; // Temporary buffer
    // Read each line from the files
    while (fgets(line, sizeof(line), files)) {
        // Trim newline characters from the end of the line
        while (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        line[40] = '/';
        // Build source & target path
        strcpy(source, rootfs_path);
        strcat(source, line+40);
        strcpy(target, destdir);
        strcat(target, line+40);
        debug("file: %s -> %s\n", source, target);
        // create parent directory if not exists
        strcpy(tmp, target);
        (void)dirname(tmp);
        create_dir(tmp);
        // move file
        status = rename(source, target);
        // set permission
        status += chmod(target, 0755);
        status += chown(target, 0,0);
        if(status != 0){
            goto free_quarantine_sync;
        }
    }

    // Read each line from the links
    while (fgets(line, sizeof(line), links)) {
        // Trim newline characters from the end of the line
        while (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        // calcuate offset of link - path seperator
        size_t offset=0;
        while(line[offset] && line[offset] != ' ') {
            offset++;
        }
        line[offset]='/';
        strcpy(source, rootfs_path);
        strcat(source, line+offset+1);
        strcpy(target, destdir);
        strcat(target, line+offset);
        debug("file: %s -> %s\n", source, target);
        // create parent directory if not exists
        strcpy(tmp, target);
        (void)dirname(tmp);
        create_dir(tmp);
        // move symlink
        status = rename(source, target);
        if(status != 0){
            goto free_quarantine_sync;
        }
    }

    // Move files
    strcpy(target, destdir);
    strcat(target, "/");
    strcat(target, STORAGE);
    strcat(target, "/files/");
    strcat(target, name);
    status += rename(files_path, target);

    // Move files
    strcpy(target, destdir);
    strcat(target, "/");
    strcat(target, STORAGE);
    strcat(target, "/links/");
    strcat(target, name);
    status += rename(links_path, target);

    // Move files
    strcpy(target, destdir);
    strcat(target, "/");
    strcat(target, STORAGE);
    strcat(target, "/metadata/");
    strcat(target, name);
    strcat(target, ".yaml");
    status += rename(metadata_path, target);

    // Cleanup: free memory
free_quarantine_sync:
    fclose(files);
    fclose(links);
    free(rootfs_path);
    free(files_path);
    free(links_path);
    free(metadata_path);
    return status;
}

static void calculate_leftovers(array* arr, const char* name){
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    array *list = array_new();
    char* files = build_string("%s/%s/files/%s", destdir, STORAGE, name);
    char* links = build_string("%s/%s/links/%s", destdir, STORAGE, name);
    char* files_new = build_string("%s/%s/quarantine/files/%s", destdir, STORAGE, name);
    char* links_new = build_string("%s/%s/quarantine/links/%s", destdir, STORAGE, name);

    char line[PATH_MAX+41];
    size_t offset = 0;
    // read files
    FILE *ffiles = fopen(files, "r");
    while (fgets(line, sizeof(line), ffiles)) {
        array_add(list, line+41);
    }
    fclose(ffiles);
    // read links
    FILE *flinks = fopen(links, "r");
    while (fgets(line, sizeof(line), flinks)) {
        for(offset=0; line[offset] && line[offset] != ' '; offset++);
        line[offset] = '\n';
        array_add(list, line);
    }
    fclose(flinks);
    // read new files
    FILE *ffiles_new = fopen(files_new, "r");
    while (fgets(line, sizeof(line), ffiles_new)) {
        array_remove(list, line+41);
    }
    fclose(ffiles_new);
    // read links
    FILE *flinks_new = fopen(links_new, "r");
    while (fgets(line, sizeof(line), flinks_new)) {
        for(offset=0; line[offset] && line[offset] != ' '; offset++);
        line[offset] = '\n';
        array_remove(list, line);
    }
    fclose(flinks_new);
    // add leftovers
    size_t len;
    array_adds(arr, array_get(list, &len));
    // free memory
    free(files);
    free(links);
    free(files_new);
    free(links_new);
    array_unref(list);
}

// Function to validate all quarantine metadata files
visible bool quarantine_validate() {
    debug("validate event");
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
            jobs_add(j, (callback)quarantine_validate_links, basename(metadatas[i]), NULL);
        }
    }

    // Run the jobs and check for failures
    jobs_run(j);
    bool status = j->failed; // Capture the failure status
    jobs_unref(j); // Unreference the job queue


    // create leftover array
    array *leftover = array_new();

    // Sync if validation sucessfully
    if(!status){
        j = jobs_new();
        // Iterate through each metadata file
        for (size_t i = 0; metadatas[i]; i++) {
            calculate_leftovers(leftover, basename(metadatas[i]));
            jobs_add(j, (callback)quarantine_sync, basename(metadatas[i]), NULL);
        }
        // Run the jobs and check for failures
        jobs_run(j);
        status = j->failed;  // Capture the failure status
        jobs_unref(j); // Unreference the job queue
    }

    // remove leftovers
    size_t len;
    char** left = array_get(leftover, &len);
    char target[PATH_MAX];
    for(size_t i=0; i < len; i++){
        strcpy(target, destdir);
        strcat(target,"/");
        strcat(target, left[i]);
        unlink(target);
        free(left[i]);
    }
    free(left);
    array_unref(leftover);


    // Reset after sync
    quarantine_reset();

    // Cleanup: free allocated memory for metadata file names
    for (size_t i = 0; metadatas[i]; i++) {
        free(metadatas[i]);
    }
    free(metadata);
    free(metadatas);
    return !status;
}


visible void quarantine_reset(){
    debug("reset event");
    // Build quarantine directory
    char* destdir = variable_get_value(global->variables, "DESTDIR");
    char* path = build_string("%s/%s/quarantine/", destdir, STORAGE);
    // remove if exists
    if(isdir(path)){
        remove_all(path);
    }
    // recreate again
    create_dir(path);
    // cleanup
    free(path);
}
