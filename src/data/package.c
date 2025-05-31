#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <data/package.h>
#include <data/build.h>

#include <utils/archive.h>
#include <utils/yaml.h>
#include <utils/error.h>
#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <core/variable.h>

#include <config.h>

visible Package* package_new() {
    // Allocate memory for a new Package structure
    Package *pkg = malloc(sizeof(Package));

    // Initialize the archive member of the Package with a new archive
    pkg->archive = archive_new();
    pkg->is_virtual = false;

    // Return the newly created Package instance
    return pkg;
}

visible void package_unref(Package *pkg){
    free(pkg->archive);
    free(pkg);
}

visible void package_load_from_file(Package* pkg, const char* path) {
    if(!pkg){
        return;
    }
    debug("Package load from file: %s\n", path);
    // Check if the specified path is a valid file
    if(!isfile(path)){
        error_add(build_string("Failed to load package archive %s", path));
        return; // Exit if the file does not exist
    }

    // 1. Load the archive from the specified file path
    archive_load(pkg->archive, path);

    // Read the metadata from the archive
    pkg->metadata = archive_readfile(pkg->archive, "metadata.yaml");
    if(pkg->metadata == NULL) {
        error_add("Failed to load metadata");
        return; // Exit if metadata loading fails
    }

    // 2. Extract relevant metadata areas
    // Check if the metadata contains an area named "ymp"
    if(yaml_has_area(pkg->metadata, "ymp")){
        pkg->metadata = yaml_get_area(pkg->metadata,"ymp"); // Get the "ymp" area
    }

    // Check if the metadata contains a "source" area
    if(yaml_has_area(pkg->metadata, "source")){
        pkg->is_source = true; // Mark package as a source package
        pkg->metadata = yaml_get_area(pkg->metadata, "source"); // Get the "source" area
    } else if(yaml_has_area(pkg->metadata, "package")){
        pkg->is_source = false; // Mark package as a regular package
        pkg->metadata = yaml_get_area(pkg->metadata, "package"); // Get the "package" area
    } else {
        error_add("Metadata is invalid"); // Handle invalid metadata
    }
    package_load_from_metadata(pkg, pkg->metadata, pkg->is_source);
}

visible void package_load_from_metadata(Package* pkg, const char* metadata, bool is_source){
    if(!pkg){
        return;
    }
    pkg->is_source = is_source;
    pkg->metadata = metadata;
    if(!pkg->is_source && !pkg->is_virtual){
        // 3. Read the list of files from the archive
        pkg->files = archive_readfile(pkg->archive, "files");
        if(pkg->files == NULL) {
            error_add("Failed to load file list"); // Handle failure to load file list
            return; // Exit if file list loading fails
        }

        // Read the list of symlinks from the archive
        pkg->links = archive_readfile(pkg->archive, "links");
        if(pkg->links == NULL) {
            error_add("Failed to load link list"); // Handle failure to load link list
            return; // Exit if link list loading fails
        }
    }

    // Read the package information from the archive
    pkg->name = yaml_get_value(pkg->metadata, "name");
    pkg->version = yaml_get_value(pkg->metadata, "version");
    pkg->release = atoi(yaml_get_value(pkg->metadata, "release"));
    int dep_count = 0;
    pkg->dependencies = yaml_get_array(pkg->metadata, "dependencies", &dep_count);
    debug("package:%s - %s\n", pkg->name, pkg->version);

}

// Function to extract a package
visible bool package_extract(Package* pkg) {
    // Check if the package pointer is NULL
    if(!pkg) {
        warning("%s\n", "Invalid package!");
        return false; // Return false if the package is invalid
    }

    // Check if the package archive is NULL
    if(pkg->archive == NULL) {
        warning("%s\n", "Invalid package archive!");
        return false; // Return false if the archive is invalid
    }
    info("Package extract: %s\n", pkg->name);

    // Get the destination directory from global variables
    char* destdir = variable_get_value(global->variables, "DESTDIR");

    // Build the root filesystem path for quarantine
    char* rootfs = build_string("%s/%s/quarantine/rootfs", destdir, STORAGE);
    char* metadata_dir = build_string("%s/../metadata", rootfs);
    char* files_dir = build_string("%s/../metadata", rootfs);
    char* links_dir = build_string("%s/../metadata", rootfs);

    // If package is source, build instead of extract
    if(pkg->is_source){
        // Extract source package to the cache
        char* cache = build_string("%s/cache/%s-%s", BUILD_DIR, pkg->name, pkg->version);
        archive_set_target(pkg->archive, cache);
        archive_extract_all(pkg->archive);
        // Build source package
        char* build = build_binary_from_path(cache);
        printf("%s\n", build);
        return true;
    }

    // Build a temporary directory path for extraction
    char* tmpdir = build_string("%s/../tmp/%s", rootfs, pkg->name);

    // Create necessary directories for the extraction process
    create_dir(rootfs);
    create_dir(metadata_dir);
    create_dir(files_dir);
    create_dir(files_dir);
    create_dir(tmpdir);

    // Set the target for the archive extraction to the temporary directory
    archive_set_target(pkg->archive, tmpdir);

    // Extract all contents of the package archive
    archive_extract_all(pkg->archive);

    size_t i = 0; // Initialize index for file listing
    char** files = listdir(tmpdir); // List files in the temporary directory

    // Loop through the files in the temporary directory
    while(files[i]) {
        // Skip if the current item is not a file
        if(!isfile(files[i])) {
            i++;
            continue;
        }

        // Check if the file name starts with "data."
        if(startswith(files[i], "data.")) {
            // Build the full path for the data file
            char* file = build_string("%s/%s", tmpdir, files[i]);

            // Calculate the SHA1 hash of the data file
            char* hash = calculate_sha1(file);

            // Get the expected hash from the package metadata
            char* yaml_hash = yaml_get_value(pkg->metadata, "archive-hash");

            // Compare the calculated hash with the expected hash
            if(!iseq(hash, yaml_hash)) {
                warning("%s Excepted %s <> Received %s\n", "Package archive hash is wrong!", hash, yaml_hash);
                free(hash);
                free(yaml_hash);
                free(file);
                return false; // Return false if hashes do not match
            }
            debug("Package archive hash: %s\n", hash);

            // Create a new archive object for the data file
            Archive *data = archive_new();
            archive_load(data, file); // Load the data file into the archive

            // Set the target for the data extraction to the root filesystem
            archive_set_target(data, rootfs);
            archive_extract_all(data); // Extract all contents of the data archive

            free(hash);
            free(yaml_hash);
            free(file);
            free(data); // Free the archive object
            break; // Exit the loop after processing the data file
        }
        i++; // Move to the next file
    }

    free(files); // Free the list of files

    // Rename and move the metadata, files, and links to their respective directories
    #define rename_free(A,B) {char *a=A; char *b=B; rename(a,b) ; free(a) ; free(b);}
    rename_free(build_string("%s/metadata.yaml", tmpdir), build_string("%s/../metadata/%s.yaml", rootfs, pkg->name));
    rename_free(build_string("%s/files", tmpdir), build_string("%s/../files/%s", rootfs, pkg->name));
    rename_free(build_string("%s/links", tmpdir), build_string("%s/../links/%s", rootfs, pkg->name));

    // Cleanup
    free(tmpdir);
    free(rootfs);
    free(metadata_dir);
    free(files_dir);
    free(links_dir);

    return true; // Return true if extraction was successful
}

