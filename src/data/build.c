#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <libgen.h>
#include <errno.h>
#include <sched.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <core/operations.h>

#include <utils/string.h>
#include <utils/file.h>
#include <utils/hash.h>
#include <utils/fetcher.h>
#include <utils/archive.h>
#include <utils/yaml.h>

#include <data/build.h>
#include <config.h>

#define UNSHARE_FLAGS (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNET)


visible char* ympbuild_get_value(ympbuild* ymp, const char* name) {
    char* command = build_string(
    "exec <&-\n"
    "{\n%s\n} &>/dev/null\n"
    "echo -n ${%s}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput_unshare(args, UNSHARE_FLAGS | CLONE_NEWPID));
    debug("variable: %s -> %s\n", name, output);
    free(command);
    return output;
}

visible char** ympbuild_get_array(ympbuild* ymp, const char* name){
    char* command = build_string(
    "exec <&-\n"
    "{\n%s\n} &>/dev/null\n"
    "echo -n ${%s[@]}", ymp->ctx, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    char* output = strip(getoutput_unshare(args, UNSHARE_FLAGS | CLONE_NEWPID));
    debug("variable: %s -> %s\n", name, output);
    free(command);
    return split(output," ");

}

visible int ympbuild_run_function(ympbuild* ymp, const char* name) {
    char* command = build_string(
        "exec <&-\n"
        "set +e ; %s\n"
        "%s\n"
        "set -e \n"
        "if declare -F %s ; then\n"
        "    %s\n"
        "fi", ymp->header, ymp->ctx, name, name);
    char* args[] = {"/bin/bash", "-c", command, NULL};
    pid_t pid = fork();
    if(pid == 0){
        if(chdir(ymp->path) < 0){
            free(command);
            return -1;
        }
        char* envs[] = {
            build_string("PATH=/usr/bin:/usr/sbin:/bin:/sbin/%s", ymp->path),
            build_string("HOME=%s", ymp->path),
            NULL
        };
        if (unshare(UNSHARE_FLAGS) < 0) {
            exit(1);
        }
        if(sethostname("sandbox",7) < 0){
            exit(1);
        }
        execve(args[0], args, envs);
        free(command);
        return -1;
    } else {
        int status = 0;
        (void)waitpid(pid, &status, 0);
        free(command);
        return status;
    }
}

static char* hash_types[] = {"sha512sums", "sha256sums", "sha1sums", "md5sums", NULL};

static bool get_resource(const char* resource_path, const char* resource_name, size_t resource_type, const char* source_url, const char* expected_hash) {
    debug("Source: %s %s\n", source_url, expected_hash);

    // Get the file name from the source URL
    char* source_file_name = basename((char*)source_url);

    // Construct the target cache directory path
    char* cache_directory = build_string("%s/cache/%s", BUILD_DIR, resource_name);
    char* target_file_path = build_string("%s/%s", cache_directory, source_file_name);

    bool operation_status = true;

    // Check if the target file already exists
    if (!isfile(target_file_path)) {
        // Download or Copy the resource
        create_dir(cache_directory);

        char* local_file_path = build_string("%s/%s", resource_path, source_url);

        if (isfile(local_file_path)) {
            free(target_file_path);
            target_file_path = build_string("%s/%s", cache_directory, source_file_name);
            operation_status = copy_file(local_file_path, target_file_path);
        } else {
            operation_status = fetch(source_url, target_file_path);
        }

        free(local_file_path);
    }

    // Check the hash of the downloaded or copied file
    char* actual_hash = calculate_hash(resource_type, target_file_path);

    if (iseq((char*)expected_hash, "SKIP")) {
        warning("Hash control disabled for: %s\n", source_file_name);
    } else if (!iseq(actual_hash, (char*)expected_hash)) {
        print("Archive hash is invalid:\n  -> Expected: %s\n  -> Received: %s\n", expected_hash, actual_hash);
        return false;
    }

    // Cleanup
    free(cache_directory);
    free(target_file_path);

    return operation_status;
}


static char* actions[] = {"prepare", "setup", "build", "package", NULL};

static char** get_uses(ympbuild *ymp) {
    // Retrieve the value of the "build:use" variable from the global variable store
    char* uses = variable_get_value(global->variables, "build:use");

    // Create a new array to hold the uses
    array *flag = array_new();

    // Check if the retrieved uses string is not empty
    if (strlen(uses) > 0) {
        // Split the uses string by spaces and add the resulting tokens to the flag array
        array_adds(flag, split(uses, " "));
    } else {
        // If the uses string is empty, add "all" to the flag array
        array_add(flag, "all");
    }

    // Check if the flag array contains "all"
    if (array_has(flag, "all")) {
        // Remove "all" from the flag array
        array_remove(flag, "all");
        // Add the standard uses from the ympbuild structure to the flag array
        array_adds(flag, ympbuild_get_array(ymp, "uses"));
    }

    // Check if the flag array contains "extra"
    if (array_has(flag, "extra")) {
        // Remove "extra" from the flag array
        array_remove(flag, "extra");
        // Add the extra uses from the ympbuild structure to the flag array
        array_adds(flag, ympbuild_get_array(ymp, "uses_extra"));
    }

    // Get the contents of the flag array as a char** and retrieve its length
    size_t len = 0;
    char** ret = array_get(flag, &len);

    // Unreference the flag array to manage memory
    array_unref(flag);

    // Return the array of uses
    return ret;
}

static void configure_header(ympbuild *ymp) {
    ymp->header = readfile(":/ympbuild-header.sh");
    ymp->header = str_replace(ymp->header, "@buildpath@", ymp->path);
    ymp->header = str_replace(ymp->header, "@CC@", variable_get_value(global->variables, "build:cc"));
    ymp->header = str_replace(ymp->header, "@CXX@", variable_get_value(global->variables, "build:cxx"));
    ymp->header = str_replace(ymp->header, "@CFLAGS@", variable_get_value(global->variables, "build:cflags"));
    ymp->header = str_replace(ymp->header, "@CXXFLAGS@", variable_get_value(global->variables, "build:cxxflags"));
    ymp->header = str_replace(ymp->header, "@LDFLAGS@", variable_get_value(global->variables, "build:ldflags"));
    ymp->header = str_replace(ymp->header, "@APIKEY@", variable_get_value(global->variables, "build:token"));
    ymp->header = str_replace(ymp->header, "@DISTRODIR@", DISTRODIR);
    char** flag = get_uses(ymp);
    for(size_t i=0; flag[i];i++){
        char* new_header = build_string("%s\ndeclare -r use_%s=31\n", ymp->header, flag[i]);
        free(ymp->header);
        ymp->header = new_header;
    }
    free(flag);
}

static void generate_links_files(const char* path) {
    // Construct the root filesystem path by appending "/output" to the provided path
    char* rootfs = build_string("%s/output", path);

    // Find all inodes (files and symlinks) in the root filesystem
    char** inodes = find(rootfs);

    // Create new arrays to hold file and symlink information
    array *files = array_new();
    array *links = array_new();

    // Iterate through the inodes to process each one
    for (size_t i = 0; inodes[i]; i++) {
        // Check if the current inode is a symlink
        if (issymlink(inodes[i])) {
            debug("add symlink: %s\n", inodes[i]);
            // Add the symlink information to the links array
            array_add(links, build_string("%s %s\n", sreadlink(inodes[i]), inodes[i] + strlen(rootfs)));
        }
        // Check if the current inode is a regular file
        else if (isfile(inodes[i])) {
            debug("add file: %s\n", inodes[i]);
            // Calculate the SHA1 hash of the file
            char* hash = calculate_sha1(inodes[i]);
            // Add the file hash and path to the files array
            array_add(files, build_string("%s %s\n", hash, inodes[i] + strlen(rootfs) + 1));
            // Free the memory allocated for the hash
            free(hash);
        }
        // Free the memory allocated for the current inode
        free(inodes[i]);
    }

    // Construct paths for the output files
    char* files_path = build_string("%s/files", path);
    char* links_path = build_string("%s/links", path);

    // Write the contents of the files and links arrays to their respective files
    writefile(files_path, array_get_string(files));
    writefile(links_path, array_get_string(links));

    // Cleanup: free allocated memory and unreference arrays
    free(inodes);
    free(links_path);
    free(files_path);
    array_unref(files);
    array_unref(links);
}


static char* metadata_vars[] = {"name", "version", "description", "release", NULL};
static char* source_arrs[] = {"depends", "makedepends", "arch", "provides", "replaces", "source", NULL};

static char* getArch() {
    struct utsname buffer;
    errno = 0;
    if (uname(&buffer) < 0) {
        perror("uname");
    }
    return strdup(buffer.machine);
}

static void generate_metadata(ympbuild *ymp, bool is_source) {
    // Create a new array to hold the metadata lines
    array *a = array_new();

    // Add the initial "ymp:" line to the metadata
    array_add(a, "ymp:\n");

    // Add either "source:" or "package:" based on the is_source flag
    if (is_source) {
        array_add(a, "  source:\n");
    } else {
        array_add(a, "  package:\n");
    }

    // Add common metadata variables
    for (size_t i = 0; metadata_vars[i]; i++) {
        array_add(a, build_string("    %s: %s\n", metadata_vars[i], ympbuild_get_value(ymp, metadata_vars[i])));
    }

    // If not a source, add package-specific metadata
    if (!is_source) {
        array_add(a, build_string("    arch: %s\n", getArch()));

        // Create an array to hold dependencies
        array *deps = array_new();
        array_adds(deps, ympbuild_get_array(ymp, "depends"));

        // Get the use flags and add their dependencies
        char** flag = get_uses(ymp);
        for (size_t i = 0; flag[i]; i++) {
            array_adds(deps, ympbuild_get_array(ymp, build_string("%s_depends", flag[i])));
        }

        // Add the dependencies section to the metadata
        array_add(a, "    depends:\n");
        size_t len = 0;
        char** depends = array_get(deps, &len);
        for (size_t i = 0; depends[i] && strlen(depends[i]) > 0; i++) {
            array_add(a, build_string("      - %s\n", depends[i]));
        }

        // Free allocated memory for dependencies and flags
        free(depends);
        free(flag);
    } else {
        // If it's a source, add source-specific metadata
        for (size_t i = 0; source_arrs[i]; i++) {
            char** items = ympbuild_get_array(ymp, source_arrs[i]);
            if (items[0] && strlen(items[0]) > 0) {
                array_add(a, build_string("    %s:\n", source_arrs[i]));
                for (size_t j = 0; items[j]; j++) {
                    array_add(a, build_string("      - %s\n", items[j]));
                }
            }
        }

        // Add use flags
        array *uses = array_new();
        array_adds(uses, ympbuild_get_array(ymp, "uses"));
        array_adds(uses, ympbuild_get_array(ymp, "uses_extra"));

        size_t len = 0;
        char** flags = array_get(uses, &len);
        if (strlen(flags[0]) > 0) {
            array_add(a, "    use-flags:\n");
        }
        for (size_t i = 0; flags[i] && strlen(flags[i]) > 0; i++) {
            array_add(a, build_string("      - %s:\n", flags[i]));
        }

        // Add dependencies for each use flag
        for (size_t i = 0; flags[i] && strlen(flags[i]) > 0; i++) {
            array_add(a, build_string("    %s-depends:\n", flags[i]));
            char** deps = ympbuild_get_array(ymp, build_string("%s_depends", flags[i]));
            for (size_t j = 0; deps[j]; j++) {
                array_add(a, build_string("      - %s\n", deps[j]));
                free(deps[j]); // Free each dependency string
            }
            free(deps); // Free the array of dependencies
            free(flags[i]); // Free each flag string
        }
        free(flags); // Free the array of flags
        array_unref(uses); // Unreference the uses array
    }

    // Convert the array to a string and write it to the metadata file
    char* ret = array_get_string(a);
    array_unref(a); // Unreference the metadata array
    writefile(build_string("%s/metadata.yaml", ymp->path), ret); // Write to the specified file
}


visible char* build_source_from_path(const char* path) {
    // Check if the global context is initialized
    if (!global) {
        print("Error: ymp global missing!\n");
        return NULL; // Return NULL if global context is missing
    }

    // Construct the path to the ympbuild file
    char* ympfile = build_string("%s/ympbuild", path);

    // Check if the ympbuild file exists
    if (!isfile(ympfile)) {
        free(ympfile); // Free the allocated string if the file does not exist
        return NULL; // Return NULL if the file is not found
    }

    // Allocate memory for a new ympbuild structure
    ympbuild *ymp = malloc(sizeof(ympbuild));

    // Read the contents of the ympbuild file into the context
    ymp->ctx = readfile(ympfile);

    // Define variables for name and version from the ympbuild context
    char* name = ympbuild_get_value(ymp, "name");
    char* version = ympbuild_get_value(ymp, "version");

    // Create a source cache directory path based on name and version
    char* src_cache = build_string("%s/cache/%s-%s/", BUILD_DIR, name, version);
    create_dir(src_cache); // Create the directory for the source cache

    // Generate source metadata
    ymp->path = src_cache;
    generate_metadata(ymp, true);

    // Detect hash type
    char** hashs = NULL;
    size_t hash_type = 0;
    for (hash_type = 0; hash_types[hash_type]; hash_type++) {
        hashs = ympbuild_get_array(ymp, hash_types[hash_type]);
        if (strlen(hashs[0]) > 0) {
            break; // Break if a valid hash is found
        }
        free(hashs); // Free the hash array if not used
    }

    // Copy the ympbuild file to the source cache
    char* target = build_string("%s/ympbuild", src_cache);
    copy_file(ympfile, target); // Copy the file to the target location
    free(target); // Free the target path string

    // Copy resources based on the source array and hash
    char** sources = ympbuild_get_array(ymp, "source");
    for (size_t i = 0; sources[i] && hashs[i]; i++) {
        // Get the resource and check for success
        if (!get_resource(path, build_string("%s-%s", name, version), hash_type, sources[i], hashs[i])) {
            return NULL; // Return NULL if resource retrieval fails
        }
    }

    // Free allocated resources
    free(ymp->ctx);
    free(ymp);

    // Return the path of the source cache
    return src_cache;
}

visible char *build_binary_from_path(const char* path) {
    // Check if the global context is initialized
    if (!global) {
        print("Error: ymp global missing!\n");
        return NULL; // Return NULL if global context is missing
    }

    // Construct the path to the ympbuild file
    char* ympfile = build_string("%s/ympbuild", path);

    // Check if the ympbuild file exists
    if (!isfile(ympfile)) {
        free(ympfile); // Free the allocated string if the file does not exist
        return NULL; // Return NULL if the file is not found
    }

    // Allocate memory for a new ympbuild structure
    ympbuild *ymp = malloc(sizeof(ympbuild));

    // Read the contents of the ympbuild file into the context
    ymp->ctx = readfile(ympfile);

    // Create a build path based on the MD5 hash of the ympfile
    char* build_id = calculate_md5(ympfile);
    ymp->path = build_string("%s/%s", BUILD_DIR, build_id);

    // Create the directory for the build path
    if(isdir(ymp->path)){
        remove_all(ymp->path);
    }
    create_dir(ymp->path);

    // Configure the header for the build
    configure_header(ymp);

    // Find source files in the specified path
    char** src_files = find(path);

    // Create a new archive object
    Archive *a = archive_new();

    // Iterate through the source files
    for (size_t i = 0; src_files[i]; i++) {
        debug("Copy / Extract %s\n", src_files[i]);

        // Check if the current source file is an archive
        if (archive_is_archive(a, src_files[i])) {
            // Load the archive and set the target path for extraction
            archive_load(a, src_files[i]);
            archive_set_target(a, ymp->path);
            archive_extract_all(a); // Extract all contents of the archive
        } else {
            // If it's a regular file, copy it to the build path
            char* src_name = src_files[i] + strlen(path); // Get the relative path
            char* target_path = build_string("%s/%s", ymp->path, src_name);
            copy_file(src_files[i], target_path); // Copy the file
            free(target_path); // Free the target path string
        }
    }

    // Execute actions defined in the actions array
    int status = 0;
    for (size_t i = 0; actions[i]; i++) {
        status = ympbuild_run_function(ymp, actions[i]);
        if (status != 0) {
            free(ympfile); // Free the ympfile string on error
            return NULL; // Return NULL if any action fails
        }
    }

    // Generate links and metadata files for the build
    generate_links_files(ymp->path);
    generate_metadata(ymp, false);

    // Duplicate the build path string to return
    char* ret = strdup(ymp->path);

    // Cleanup: free allocated resources
    free(ymp->ctx);
    free(ymp->path);
    free(ymp);
    free(build_id);
    free(ympfile);

    // Return the path of the built binary
    return ret;
}


visible bool build_from_path(const char* path) {
    // Create the source from the specified path
    char* cache = build_source_from_path(path);
    print("Source created at: %s\n", cache); // Print the location of the created source

    if (cache != NULL) {
        return NULL;
    }
    // Build the binary from the created source
    char* build = build_binary_from_path(cache);
    print("Binary created at: %s\n", build); // Print the location of the created binary

    // Return true if both the binary and cache are successfully created, otherwise false
    return (build != NULL && cache != NULL);
}

static char* pwd(){
   char cwd[1024]; // Buffer to store the directory path
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
      debug("Current working directory: %s\n", cwd);
   } else {
      perror("getcwd() error"); // Prints error if getcwd() fails
   }
   return strdup(cwd);
}

visible char* create_package(const char* path) {
    // Get the current working directory
    char* curdir = pwd();

    // Construct the path for the metadata file and the output package
    char* metadata_file = build_string("%s/metadata.yaml", path);
    char* ret = build_string("%s/package.zip", path);

    // Read the contents of the metadata file
    char* metadata = readfile(metadata_file);

    // Check if the metadata file exists
    if (!isfile(metadata_file)) {
        print("Failed to find %s\n", metadata_file);
        return NULL; // Return NULL if the file is not found
    }

    // Free the metadata file path string
    free(metadata_file);

    // Change the current directory to the specified path
    if (chdir(path) < 0) {
        print("Failed to change directory\n");
        return NULL; // Return NULL if changing directory fails
    }

    // Check if the metadata is valid and contains the "ymp" area
    if (!yaml_has_area(metadata, "ymp")) {
        print("Invalid metadata\n");
        return NULL; // Return NULL if the metadata is invalid
    }

    // Get the "ymp" area from the metadata
    metadata = yaml_get_area(metadata, "ymp");

    // If the "source" area exists in the metadata, create a package
    if (yaml_has_area(metadata, "source")) {
        Archive *a = archive_new(); // Create a new archive object
        archive_load(a, ret); // Load the package file
        archive_set_type(a, "zip", "none"); // Set the archive type to ZIP

        // Find all files in the specified path
        char** files = find(path);
        for (size_t i = 0; files[i]; i++) {
            // Add each file to the archive, adjusting the path
            archive_add(a, files[i] + strlen(path) + 1);
        }

        // Create the archive
        archive_create(a);

        // Free the archive object and the list of files
        free(a);
        free(files);
    } else if (yaml_has_area(metadata, "package")) {
        // Create a new archive object for packaging files
        Archive *a = archive_new(); 

        // Load the specified TAR.GZ package file into the archive object
        archive_load(a, build_string("%s/data.tar.gz", path)); 

        // Set the archive type to TAR with GZIP compression
        archive_set_type(a, "tar", "gzip"); 

        // Change the current working directory to the 'output' directory
        if (chdir("output") < 0) {
            print("Failed to change directory to 'output'\n");
            return NULL; // Return NULL if changing the directory fails
        }

        // Retrieve a list of all files in the specified path
        char** files = find(".");

        // Iterate through the list of files and add each one to the archive
        for (size_t i = 0; files[i]; i++) {
            // Add each file to the archive, adjusting the path to exclude the base directory
            archive_add(a, files[i] + strlen(path) + 1);
        }

        // Create the archive with the added files
        archive_create(a);

        // Free the memory
        free(files);
        free(a);

        // Change the current working directory back to the original specified path
        if (chdir(path) < 0) {
            print("Failed to change directory back to '%s'\n", path);
            return NULL; // Return NULL if changing the directory fails
        }

        // Create a new archive object for the final package
        a = archive_new(); 

        // Load the previously created package file into the new archive object
        archive_load(a, ret); 

        // Set the archive type to ZIP with no compression
        archive_set_type(a, "zip", "none"); 

        // Add necessary files to the ZIP archive
        archive_add(a, "metadata.yaml"); // Add metadata file
        archive_add(a, "files");          // Add directory containing files
        archive_add(a, "links");          // Add directory containing links
        archive_add(a, "data.tar.gz");    // Add the previously created TAR.GZ file

        // Create the final ZIP archive with the added files
        archive_create(a);

        // Free the archive object after use
        free(a);

    }

    // Change back to the original directory
    if (chdir(curdir) < 0) {
        print("Failed to change directory\n");
        return NULL; // Return NULL if changing back fails
    }

    // Return the path of the created package
    return ret;
}

