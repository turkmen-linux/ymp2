#include <stdlib.h>
#include <stdio.h>
#include <data/package.h>
#include <utils/archive.h>
#include <utils/yaml.h>

#include <utils/error.h>
#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <core/variable.h>

visible Package* package_new() {
    // Allocate memory for a new Package structure
    Package *pkg = malloc(sizeof(Package));

    // Initialize the archive member of the Package with a new archive
    pkg->archive = archive_new();

    // Initialize the errors member with a new array to store error messages
    pkg->errors = array_new();

    // Return the newly created Package instance
    return pkg;
}


visible void package_load_from_file(Package* pkg, const char* path) {
    // Check if the specified path is a valid file
    if(!isfile(path)){
        error_add(pkg->errors, build_string("Failed to load package archive %s", path));
        return; // Exit if the file does not exist
    }

    // 1. Load the archive from the specified file path
    archive_load(pkg->archive, path);

    // Read the metadata from the archive
    pkg->metadata = archive_readfile(pkg->archive, "metadata.yaml");
    if(pkg->metadata == NULL) {
        error_add(pkg->errors, "Failed to load metadata");
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
        error_add(pkg->errors, "Metadata is invalid"); // Handle invalid metadata
    }

    if(!pkg->is_source){
        // 3. Read the list of files from the archive
        pkg->files = archive_readfile(pkg->archive, "files");
        if(pkg->files == NULL) {
            error_add(pkg->errors, "Failed to load file list"); // Handle failure to load file list
            return; // Exit if file list loading fails
        }

        // Read the list of symlinks from the archive
        pkg->links = archive_readfile(pkg->archive, "links");
        if(pkg->links == NULL) {
            error_add(pkg->errors, "Failed to load link list"); // Handle failure to load link list
            return; // Exit if link list loading fails
        }
    }

    // Read the package information from the archive
    pkg->name = yaml_get_value(pkg->metadata, "name");
    pkg->version = yaml_get_value(pkg->metadata, "version");
    pkg->release = atoi(yaml_get_value(pkg->metadata, "release"));
    int dep_count = 0;
    pkg->dependencies = yaml_get_array(pkg->metadata, "dependencies", &dep_count);
}

visible bool package_extract(Package* pkg){
    if(pkg == NULL){
        warning("%s\n", "Invalid package!");
        return false;
    }
    if(pkg->archive == NULL){
        warning("%s\n", "Invalid package archive!");
        return false;
    }
    char* destdir = variable_get_value(global->variables,"DESTDIR");
    char* tmpdir = build_string("%s/%s/quarantine/tmp/%s", destdir, get_storage(), pkg->name);
    create_dir(tmpdir);
    archive_set_target(pkg->archive, tmpdir);
    archive_extract_all(pkg->archive);
    size_t i=0;
    char** files = listdir(tmpdir);
    while(files[i]){
        if(!isfile(files[i])){
            i++;
            continue;
        }
        if(startswith(files[i],"data.")) {
            char* hash = calculate_sha1(build_string("%s/%s", tmpdir, files[i]));
            char* yaml_hash = yaml_get_value(pkg->metadata, "archive-hash");
            if(!iseq(hash, yaml_hash)){
                warning("%s Excepted %s <> Received %s\n", "Package archive hash is wrong!", hash, yaml_hash);
                return false;
            }
        }
        i++;
    }
    return true;
}