#define _GNU_SOURCE
#include <config.h>
#include <errno.h>
#include <libgen.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <core/logger.h>
#include <core/operations.h>
#include <core/ymp.h>
#include <data/build.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <utils/archive.h>
#include <utils/fetcher.h>
#include <utils/file.h>
#include <utils/gui.h>
#include <utils/hash.h>
#include <utils/sandbox.h>
#include <utils/string.h>
#include <utils/tty.h>
#include <utils/yaml.h>

visible char *ympbuild_get_value(ympbuild *ymp, const char *name) {
    char *command = build_string(
        "exec <&-\n"
        "{\n%s\n} &>/dev/null\n"
        "echo -n ${%s}",
        ymp->ctx, name);
    char *args[] = { "/bin/bash", "-c", command, NULL };
    char *raw = getoutput_unshare(args, CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNET | CLONE_NEWPID);
    char *output = strip(raw);
    free(raw);
    debug("variable: %s -> %s\n", name, output);
    free(command);
    return output;
}

visible char **ympbuild_get_array(ympbuild *ymp, const char *name) {
    char *command = build_string(
        "exec <&-\n"
        "{\n%s\n} &>/dev/null\n"
        "echo -n ${%s[@]}",
        ymp->ctx, name);
    char *args[] = { "/bin/bash", "-c", command, NULL };
    char *raw = getoutput_unshare(args, CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNET | CLONE_NEWPID);
    char *output = strip(raw);
    free(raw);
    debug("variable: %s -> %s\n", name, output);
    free(command);
    char **ret = split(output ? output : "", " ");
    free(output);
    return ret;
}

visible char *ympbuild_package_filename(const char *path) {
    char *ympfile = build_string("%s/ympbuild", path);
    // Allocate memory for a new ympbuild structure
    ympbuild *ymp = calloc(1, sizeof(ympbuild));
    if (!ymp) {
        free(ympfile);
        return NULL;
    }

    // Read the contents of the ympbuild file into the context
    ymp->ctx = readfile(ympfile);

    // Define variables for name and version from the ympbuild context
    char *name = ympbuild_get_value(ymp, "name");
    char *version = ympbuild_get_value(ymp, "version");
    char *release = ympbuild_get_value(ymp, "release");
    char *ret = NULL;
    size_t total_size = strlen(name) + strlen(version) + strlen(release) + strlen(ARCH) + 10;
    ret = calloc(total_size, sizeof(char));
    if (!ret) {
        ret = NULL;
        goto ympbuild_package_filename_free;
    }
    snprintf(ret, total_size, "%s_%s_%s_%s.ymp", name, version, release, ARCH);
ympbuild_package_filename_free:
    // free memory
    free(name);
    free(version);
    free(release);
    free(ymp->ctx);
    free(ymp);
    free(ympfile);
    return ret;
}

visible char *ympbuild_source_filename(const char *path) {
    char *ympfile = build_string("%s/ympbuild", path);
    // Allocate memory for a new ympbuild structure
    ympbuild *ymp = calloc(1, sizeof(ympbuild));
    if (!ymp) {
        free(ympfile);
        return NULL;
    }

    // Read the contents of the ympbuild file into the context
    ymp->ctx = readfile(ympfile);

    // Define variables for name and version from the ympbuild context
    char *name = ympbuild_get_value(ymp, "name");
    char *version = ympbuild_get_value(ymp, "version");
    char *release = ympbuild_get_value(ymp, "release");
    char *ret = NULL;
    size_t total_size = strlen(name) + strlen(version) + strlen(release) + 20;
    ret = calloc(total_size, sizeof(char));
    if (!ret) {
        ret = NULL;
        goto ympbuild_source_filename_free;
    }
    snprintf(ret, total_size, "%s_%s_%s_source.ymp", name, version, release);
ympbuild_source_filename_free:
    // free memory
    free(name);
    free(version);
    free(release);
    free(ymp->ctx);
    free(ymp);
    free(ympfile);
    return ret;
}

visible int ympbuild_check(char *ympfile) {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[] = { "/bin/bash", "-n", ympfile, NULL };
        char *envs[] = { "PATH=/usr/bin:/usr/sbin:/bin:/sbin/", NULL };
        execve(args[0], args, envs);
        exit(1);
    } else {
        int status = 0;
        (void) waitpid(pid, &status, 0);
        return status;
    }
}

static void sandbox_build(ympbuild *ymp) {
    if (get_bool("no-sandbox")) {
        return;
    }
    // Create and configure the sandbox.
    char *uuid = generate_uuid();
    sandbox_handle_t *handle = sandbox_new();
    if (!handle) {
        exit(EXIT_FAILURE);
    }
    sandbox_configure_hostname(handle, uuid);
    sandbox_configure_bind(handle, "tmpfs", "/tmp");

    char *dirs[] = {
        "/dev", "/sys", "/proc", "/usr",
        "/lib", "/bin", "/etc", "/lib64",
        "/lib32", "/libx32", "/var", NULL

    };
    for (size_t i = 0; dirs[i]; i++) {
        sandbox_configure_bind(handle, dirs[i], dirs[i]);
    }

    sandbox_configure_bind(handle, ymp->path, ymp->path);

    sandbox_configure_network(handle, false);

    // Apply the sandbox and run the command inside it.
    sandbox_apply(handle);
    sandbox_unref(handle);
    free(uuid);
}

visible int ympbuild_run_function(ympbuild *ymp, const char *name) {
    enable_raw_mode();
    pid_t pid = fork();
    if (pid == 0) {
        char *command = build_string(
            "exec <&-\n"
            "set +e ; %s\n"
            "%s\n"
            "set -e \n"
            "declare -r ACTION=%s\n"
            "if declare -F %s ; then\n"
            "    for type in ${buildtypes[@]} main; do\n"
            "        export BUILDTYLPE=$type\n"
            "        %s\n"
            "    done\n"
            "fi",
            ymp->header, ymp->ctx, name, name, name);
        char *args[] = { "/bin/bash", "-c", command, NULL };
        char *envs[] = {
            build_string("PATH=%s:/usr/bin:/usr/sbin:/bin:/sbin/", ymp->path),
            build_string("HOME=%s", ymp->path),
            NULL
        };
        sandbox_build(ymp);
        if (chdir(ymp->path) < 0) {
            warning(_("Build directory is invalid or inaccessible.\n"));
            free(command);
            return -1;
        }
        execve(args[0], args, envs);
        warning(_("Failed to execute build command.\n"));
        free(command);
        exit(1);
    } else {
        disable_raw_mode();
        int status = 0;
        (void) waitpid(pid, &status, 0);
        return status;
    }
}

static void binary_process(const char *path) {
    debug("Binary process: %s\n", path);
    // Construct the root filesystem path by appending "/output" to the provided path
    char *rootfs = build_string("%s/output", path);
    if (!rootfs) {
        return;
    }

    // Find all inodes (files and symlinks) in the root filesystem
    char **inodes = find(rootfs);
    if (!inodes) {
        free(rootfs);
        return;
    }
    for (size_t i = 0; inodes[i]; i++) {
        if (endswith(inodes[i], ".a")) {
            free(inodes[i]);
            continue;
        }
        if (!is_elf(inodes[i])) {
            free(inodes[i]);
            continue;
        }
        print(_("Stripping: %s\n"), inodes[i] + strlen(path) + 7);
        pid_t pid = fork();
        if (pid == 0) {
            char *cmd[] = {
                "objcopy", "-R", ".comment", "-R", ".note", "-R", ".debug_info",
                "-R", ".debug_aranges", "-R", ".debug_pubnames", "-R", ".debug_pubtypes",
                "-R", ".debug_abbrev", "-R", ".debug_line", "-R", ".debug_str",
                "-R", ".debug_ranges", "-R", ".debug_loc", inodes[i], NULL
            };
            char *envs[] = { "PATH=/usr/bin:/usr/sbin:/bin:/sbin", NULL };
            execve(cmd[0], cmd, envs);
            exit(1);
        } else {
            int status = 0;
            (void) waitpid(pid, &status, 0);
        }
        free(inodes[i]);
    }
    free(inodes);
    free(rootfs);
}

static char *hash_types[] = { "sha512sums", "sha256sums", "sha1sums", "md5sums", NULL };

static void fetch_progress_cb(const char *url, size_t downloaded, size_t total, void *userdata) {
    (void) url;
    const char *id = (const char *) userdata;
    gui_progress_update(id, downloaded, total);
}

static bool get_resource(const char *resource_path, const char *resource_name, size_t resource_type, const char *source_url, const char *expected_hash) {
    debug("Source: %s %s\n", source_url, expected_hash);

    // Get the file name from the source URL
    char *source_file_name = basename((char *) source_url);

    // Construct the target cache directory path
    char *cache_directory = build_string("%s/cache/%s", BUILD_DIR, resource_name);
    char *target_file_path = build_string("%s/%s", cache_directory, source_file_name);

    bool operation_status = true;

    // Check if the target file already exists
    if (!isfile(target_file_path)) {
        // Download or Copy the resource
        create_dir(cache_directory);

        char *local_file_path = build_string("%s/%s", resource_path, source_url);

        if (isfile(local_file_path)) {
            free(target_file_path);
            target_file_path = build_string("%s/%s", cache_directory, source_file_name);
            operation_status = copy_file(local_file_path, target_file_path);
        } else {
            if (isatty(STDOUT_FILENO)) {
                gui_progress_add(resource_name, "Downloading", source_url, 0);
                operation_status = fetch_with_progress(source_url, target_file_path, fetch_progress_cb, (void *) resource_name);
                gui_progress_remove(resource_name);
                gui_end();
            } else {
                operation_status = fetch(source_url, target_file_path);
            }
        }

        free(local_file_path);
    }

    // Check the hash of the downloaded or copied file
    char *actual_hash = calculate_hash(resource_type, target_file_path);
    if (actual_hash == NULL) {
        print(_("Hash calculation failed for: %s\n"), target_file_path);
        free(cache_directory);
        free(target_file_path);
        return false;
    }

    if (iseq((char *) expected_hash, "SKIP")) {
        warning(_("Hash verification skipped for: %s\n"), source_file_name);
    } else if (!iseq(actual_hash, (char *) expected_hash)) {
        print(_("Archive hash verification failed:\n  -> Expected: %s\n  -> Received: %s\n"), expected_hash, actual_hash);
        free(actual_hash);
        free(cache_directory);
        free(target_file_path);
        return false;
    }
    free(actual_hash);

    // Cleanup
    free(cache_directory);
    free(target_file_path);

    return operation_status;
}

static char *actions[] = { "prepare", "setup", "build", "package", NULL };

static char **get_uses(ympbuild *ymp) {
    // Retrieve the value of the "build:use" variable from the global variable store
    char *uses = variable_get_value(global->variables, "build:use");

    // Create a new array to hold the uses
    array *flag = array_new();

    // Check if the retrieved uses string is not empty
    if (strlen(uses) > 0) {
        // Split the uses string by spaces and add the resulting tokens to the flag array
        char **parts = split(uses, " ");
        if (parts) {
            array_adds(flag, parts);
            for (size_t i = 0; parts[i]; i++) {
                free(parts[i]);
            }
            free(parts);
        }
    } else {
        // If the uses string is empty, add "all" to the flag array
        array_add(flag, "all");
    }

    // Check if the flag array contains "all"
    if (array_has(flag, "all")) {
        // Remove "all" from the flag array
        array_remove(flag, "all");
        // Add the standard uses from the ympbuild structure to the flag array
        char **fuses = ympbuild_get_array(ymp, "uses");
        if (fuses) {
            array_adds(flag, fuses);
            for (size_t i = 0; fuses[i]; i++) {
                free(fuses[i]);
            }
            free(fuses);
        }
    }

    // Check if the flag array contains "extra"
    if (array_has(flag, "extra")) {
        // Remove "extra" from the flag array
        array_remove(flag, "extra");
        // Add the extra uses from the ympbuild structure to the flag array
        char **extra = ympbuild_get_array(ymp, "uses_extra");
        if (extra) {
            array_adds(flag, extra);
            for (size_t i = 0; extra[i]; i++) {
                free(extra[i]);
            }
            free(extra);
        }
    }

    // Get the contents of the flag array as a char** and retrieve its length
    size_t len = 0;
    char **ret = array_get(flag, &len);

    // Unreference the flag array to manage memory
    array_unref(flag);

    // Return the array of uses
    return ret;
}

static void configure_header(ympbuild *ymp) {
    char *uuid = generate_uuid();
    char *tmp = readfile(":/ympbuild-header.sh");
    char *old = NULL;
    ymp->header = str_replace(tmp ? tmp : "", "@buildpath@", ymp->path ? ymp->path : "");
#define header_replace(H, A, B)           \
    do {                                  \
        old = (H);                        \
        (H) = str_replace(old, (A), (B)); \
        free(old);                        \
    } while (0)
    header_replace(ymp->header, "@CC@", variable_get_value(global->variables, "build:cc"));
    header_replace(ymp->header, "@CXX@", variable_get_value(global->variables, "build:cxx"));
    header_replace(ymp->header, "@CFLAGS@", variable_get_value(global->variables, "build:cflags"));
    header_replace(ymp->header, "@CXXFLAGS@", variable_get_value(global->variables, "build:cxxflags"));
    header_replace(ymp->header, "@LDFLAGS@", variable_get_value(global->variables, "build:ldflags"));
    header_replace(ymp->header, "@APIKEY@", variable_get_value(global->variables, "build:token"));
    header_replace(ymp->header, "@UUID@", uuid ? uuid : "");
    header_replace(ymp->header, "@ARCH@", ARCH);
    header_replace(ymp->header, "@DEBARCH@", DEBARCH);
    header_replace(ymp->header, "@DISTRODIR@", DISTRODIR);
#undef header_replace
    char **flag = get_uses(ymp);
    for (size_t i = 0; flag[i]; i++) {
        char *new_header = build_string("%s\ndeclare -r use_%s=31\n", ymp->header, flag[i]);
        free(ymp->header);
        ymp->header = new_header;
        free(flag[i]);
    }
    free(flag);
    free(uuid);
}

static void generate_links_files(const char *path) {
    // Construct the root filesystem path by appending "/output" to the provided path
    char *rootfs = build_string("%s/output", path);

    // Find all inodes (files and symlinks) in the root filesystem
    char **inodes = find(rootfs);

    // Create new arrays to hold file and symlink information
    array *files = array_new();
    array *links = array_new();

    if (!inodes) {
        free(rootfs);
        array_unref(files);
        array_unref(links);
        return;
    }

    // Iterate through the inodes to process each one
    for (size_t i = 0; inodes[i]; i++) {
        // Check if the current inode is a symlink
        if (issymlink(inodes[i])) {
            debug("add symlink: %s\n", inodes[i]);
            // Add the symlink information to the links array
            char *target = sreadlink(inodes[i]);
            char *entry = build_string("%s %s\n", inodes[i] + strlen(rootfs) + 1, target ? target : "");
            free(target);
            if (entry) {
                array_add(links, entry);
                free(entry);
            }
        }
        // Check if the current inode is a regular file
        else if (isfile(inodes[i])) {
            debug("add file: %s\n", inodes[i]);
            // Calculate the SHA1 hash of the file
            char *hash = calculate_sha1(inodes[i]);
            // Add the file hash and path to the files array
            char *entry = build_string("%s %s\n", hash ? hash : "", inodes[i] + strlen(rootfs) + 1);
            if (entry) {
                array_add(files, entry);
                free(entry);
            }
            // Free the memory allocated for the hash
            free(hash);
        }
        // Free the memory allocated for the current inode
        free(inodes[i]);
    }

    // Construct paths for the output files
    char *files_path = build_string("%s/files", path);
    char *links_path = build_string("%s/links", path);

    // Write the contents of the files and links arrays to their respective files
    char *files_str = array_get_string(files);
    char *links_str = array_get_string(links);
    writefile(files_path, files_str ? files_str : "");
    writefile(links_path, links_str ? links_str : "");
    free(files_str);
    free(links_str);

    // Cleanup: free allocated memory and unreference arrays
    free(inodes);
    free(rootfs);
    free(links_path);
    free(files_path);
    array_unref(files);
    array_unref(links);
}

static char *metadata_vars[] = { "name", "version", "description", "release", NULL };
static char *source_arrs[] = { "depends", "makedepends", "arch", "provides", "replaces", "source", NULL };

static char *getArch() {
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
        char *val = ympbuild_get_value(ymp, metadata_vars[i]);
        char *line = build_string("    %s: %s\n", metadata_vars[i], val ? val : "");
        free(val);
        if (line) {
            array_add(a, line);
            free(line);
        }
    }

    char *unsafe_val = ympbuild_get_value(ymp, "unsafe");
    if (unsafe_val && strlen(unsafe_val) > 0) {
        array_add(a, "    unsafe: true\n");
    }
    free(unsafe_val);

    // If not a source, add package-specific metadata
    if (!is_source) {
        char *arch = getArch();
        char *arch_line = build_string("    arch: %s\n", arch ? arch : "");
        free(arch);
        if (arch_line) {
            array_add(a, arch_line);
            free(arch_line);
        }

        // Create an array to hold dependencies
        array *deps = array_new();
        char **base_deps = ympbuild_get_array(ymp, "depends");
        if (base_deps) {
            array_adds(deps, base_deps);
            for (size_t i = 0; base_deps[i]; i++) {
                free(base_deps[i]);
            }
            free(base_deps);
        }

        // Get the use flags and add their dependencies
        char **flag = get_uses(ymp);
        for (size_t i = 0; flag[i]; i++) {
            char *key = build_string("%s_depends", flag[i]);
            char **extra = key ? ympbuild_get_array(ymp, key) : NULL;
            free(key);
            if (extra) {
                array_adds(deps, extra);
                for (size_t k = 0; extra[k]; k++) {
                    free(extra[k]);
                }
                free(extra);
            }
        }

        // Add the dependencies section to the metadata
        array_add(a, "    depends:\n");
        size_t len = 0;
        char **depends = array_get(deps, &len);
        for (size_t i = 0; depends && depends[i] && strlen(depends[i]) > 0; i++) {
            char *line = build_string("      - %s\n", depends[i]);
            if (line) {
                array_add(a, line);
                free(line);
            }
        }

        // Free allocated memory for dependencies and flags
        if (depends) {
            for (size_t i = 0; depends[i]; i++) {
                free(depends[i]);
            }
            free(depends);
        }
        array_unref(deps);
        if (flag) {
            for (size_t i = 0; flag[i]; i++) {
                free(flag[i]);
            }
            free(flag);
        }
    } else {
        // If it's a source, add source-specific metadata
        for (size_t i = 0; source_arrs[i]; i++) {
            char **items = ympbuild_get_array(ymp, source_arrs[i]);
            if (items && items[0] && strlen(items[0]) > 0) {
                char *hline = build_string("    %s:\n", source_arrs[i]);
                if (hline) {
                    array_add(a, hline);
                    free(hline);
                }
                for (size_t j = 0; items[j]; j++) {
                    char *line = build_string("      - %s\n", items[j]);
                    if (line) {
                        array_add(a, line);
                        free(line);
                    }
                }
            }
            if (items) {
                for (size_t j = 0; items[j]; j++) {
                    free(items[j]);
                }
                free(items);
            }
        }

        // Add use flags
        array *uses = array_new();
        char **u1 = ympbuild_get_array(ymp, "uses");
        char **u2 = ympbuild_get_array(ymp, "uses_extra");
        if (u1) {
            array_adds(uses, u1);
            for (size_t i = 0; u1[i]; i++) {
                free(u1[i]);
            }
            free(u1);
        }
        if (u2) {
            array_adds(uses, u2);
            for (size_t i = 0; u2[i]; i++) {
                free(u2[i]);
            }
            free(u2);
        }

        size_t len = 0;
        char **flags = array_get(uses, &len);
        if (flags && flags[0]) {
            array_add(a, "    use-flags:\n");
        }
        for (size_t i = 0; flags && flags[i] && strlen(flags[i]) > 0; i++) {
            char *line = build_string("      - %s:\n", flags[i]);
            if (line) {
                array_add(a, line);
                free(line);
            }
        }

        // Add dependencies for each use flag
        for (size_t i = 0; flags && flags[i] && strlen(flags[i]) > 0; i++) {
            char *hline = build_string("    %s-depends:\n", flags[i]);
            if (hline) {
                array_add(a, hline);
                free(hline);
            }
            char *key = build_string("%s_depends", flags[i]);
            char **deps = key ? ympbuild_get_array(ymp, key) : NULL;
            free(key);
            for (size_t j = 0; deps && deps[j]; j++) {
                char *line = build_string("      - %s\n", deps[j]);
                if (line) {
                    array_add(a, line);
                    free(line);
                }
                free(deps[j]);  // Free each dependency string
            }
            free(deps);      // Free the array of dependencies
            free(flags[i]);  // Free each flag string
        }
        free(flags);        // Free the array of flags
        array_unref(uses);  // Unreference the uses array
    }

    // Convert the array to a string and write it to the metadata file
    char *ret = array_get_string(a);
    array_unref(a);  // Unreference the metadata array
    char *meta_path = build_string("%s/metadata.yaml", ymp->path);
    if (meta_path) {
        writefile(meta_path, ret ? ret : "");  // Write to the specified file
        free(meta_path);
    }
    free(ret);
}

visible char *build_source_from_path(const char *path) {
    // Check if the global context is initialized
    if (!global) {
        print(_("Global context not initialized. Please initialize ymp first.\n"));
        return NULL;
    }

    // Construct the path to the ympbuild file
    char *ympfile = build_string("%s/ympbuild", path);

    // Check if the ympbuild file exists
    if (!isfile(ympfile)) {
        free(ympfile);
        return NULL;
    }

    // Allocate memory for a new ympbuild structure
    ympbuild *ymp = calloc(1, sizeof(ympbuild));
    if (!ymp) {
        free(ympfile);
        return NULL;
    }

    // Read the contents of the ympbuild file into the context
    ymp->ctx = readfile(ympfile);
    if (!ymp->ctx) {
        free(ymp);
        free(ympfile);
        return NULL;
    }

    // Define variables for name and version from the ympbuild context
    char *name = ympbuild_get_value(ymp, "name");
    char *version = ympbuild_get_value(ymp, "version");
    if (!name || !version) {
        free(name);
        free(version);
        free(ymp->ctx);
        free(ymp);
        free(ympfile);
        return NULL;
    }

    // Create a source cache directory path based on name and version
    char *src_cache = build_string("%s/cache/%s-%s/", BUILD_DIR, name, version);
    if (!src_cache) {
        free(name);
        free(version);
        free(ymp->ctx);
        free(ymp);
        free(ympfile);
        return NULL;
    }
    create_dir(src_cache);  // Create the directory for the source cache

    // Generate source metadata
    ymp->path = src_cache;
    generate_metadata(ymp, true);

    // Detect hash type
    char **hashs = NULL;
    size_t hash_type = 0;
    for (hash_type = 0; hash_types[hash_type]; hash_type++) {
        hashs = ympbuild_get_array(ymp, hash_types[hash_type]);
        if (hashs && hashs[0] && strlen(hashs[0]) > 0) {
            break;  // Break if a valid hash is found
        }
        if (hashs) {
            for (size_t k = 0; hashs[k]; k++) {
                free(hashs[k]);
            }
            free(hashs);
            hashs = NULL;
        }
    }

    // Copy the ympbuild file to the source cache
    char *target = build_string("%s/ympbuild", src_cache);
    if (target) {
        copy_file(ympfile, target);  // Copy the file to the target location
        free(target);                // Free the target path string
    }

    // Copy resources based on the source array and hash
    char **sources = ympbuild_get_array(ymp, "source");
    bool resource_ok = true;
    for (size_t i = 0; sources && sources[i] && hashs && hashs[i]; i++) {
        // Get the resource and check for success
        char *rname = build_string("%s-%s", name, version);
        bool ok = rname && get_resource(path, rname, hash_type, sources[i], hashs[i]);
        free(rname);
        if (!ok) {
            resource_ok = false;
            break;
        }
    }

    // Free allocated resources
    free(name);
    free(version);
    free(ymp->ctx);
    free(ymp);
    free(ympfile);
    if (sources) {
        for (size_t i = 0; sources[i]; i++) {
            free(sources[i]);
        }
        free(sources);
    }
    if (hashs) {
        for (size_t i = 0; hashs[i]; i++) {
            free(hashs[i]);
        }
        free(hashs);
    }

    if (!resource_ok) {
        free(src_cache);
        return NULL;
    }

    // Return the path of the source cache
    return src_cache;
}

visible char *build_binary_from_path(const char *path) {
    // Check if the global context is initialized
    if (!global) {
        print(_("Error: ymp global missing!\n"));
        return NULL;  // Return NULL if global context is missing
    }

    // Construct the path to the ympbuild file
    char *ympfile = build_string("%s/ympbuild", path);
    debug("ympfile: %s\n", ympfile);

    // Check if the ympbuild file exists
    if (!isfile(ympfile)) {
        print(_("ympbuild file not found: %s\n"), ympfile);
        free(ympfile);
        return NULL;  // Return NULL if the file is not found
    }

    // Allocate memory for a new ympbuild structure
    ympbuild *ymp = calloc(1, sizeof(ympbuild));
    if (!ymp) {
        free(ympfile);
        return NULL;
    }

    // Syntax check before read
    if (ympbuild_check(ympfile) != 0) {
        print(_("Syntax error detected in ympbuild file.\n"));
        free(ympfile);
        free(ymp);
        return NULL;
    }

    // Read the contents of the ympbuild file into the context
    ymp->ctx = readfile(ympfile);
    if (!ymp->ctx) {
        free(ympfile);
        free(ymp);
        return NULL;
    }

    // Create a build path based on the MD5 hash of the ympfile
    char *build_id = calculate_md5(ympfile);
    char *tmp = build_string("%s/%s", BUILD_DIR, build_id ? build_id : "");
    // Realpath
    char *resolved = tmp ? realpath(tmp, NULL) : NULL;
    if (resolved) {
        ymp->path = resolved;
        free(tmp);
    } else {
        ymp->path = tmp;
    }
    if (!ymp->path) {
        free(ymp->ctx);
        free(ymp);
        free(build_id);
        free(ympfile);
        return NULL;
    }

    // Create the directory for the build path
    if (isdir(ymp->path)) {
        remove_all(ymp->path);
    }
    create_dir(ymp->path);

    // Configure the header for the build
    configure_header(ymp);

    // Find source files in the specified path
    char **src_files = find(path);

    // Create a new archive object
    Archive *a = archive_new();
    if (!src_files || !a) {
        if (a) {
            archive_unref(a);
        }
        if (src_files) {
            for (size_t k = 0; src_files[k]; k++) {
                free(src_files[k]);
            }
            free(src_files);
        }
        free(ymp->header);
        free(ymp->ctx);
        free(ymp->path);
        free(ymp);
        free(build_id);
        free(ympfile);
        return NULL;
    }

    // Iterate through the source files
    for (size_t i = 0; src_files[i]; i++) {
        debug("Copy / Extract %s\n", src_files[i]);

        // Check if the current source file is an archive
        if (archive_is_archive(a, src_files[i])) {
            // Load the archive and set the target path for extraction
            if (!archive_load(a, src_files[i])) {
                archive_unref(a);
                for (size_t k = 0; src_files[k]; k++) {
                    free(src_files[k]);
                }
                free(src_files);
                free(ymp->header);
                free(ymp->ctx);
                free(ymp->path);
                free(ymp);
                free(build_id);
                free(ympfile);
                return NULL;
            }
            archive_set_target(a, ymp->path);
            archive_extract_all(a);  // Extract all contents of the archive
        } else {
            // If it's a regular file, copy it to the build path
            char *src_name = src_files[i] + strlen(path);  // Get the relative path
            char *target_path = build_string("%s/%s", ymp->path, src_name);
            copy_file(src_files[i], target_path);  // Copy the file
            free(target_path);                     // Free the target path string
        }
        free(src_files[i]);
        src_files[i] = NULL;
    }
    free(src_files);
    src_files = NULL;

    // Execute actions defined in the actions array
    for (size_t i = 0; actions[i]; i++) {
        debug("ymp run function: %s %s\n", actions[i], ympfile);
        int status = ympbuild_run_function(ymp, actions[i]);
        if (status != 0) {
            archive_unref(a);
            free(ymp->header);
            free(ymp->ctx);
            free(ymp->path);
            free(ymp);
            free(build_id);
            free(ympfile);
            return NULL;  // Return NULL if any action fails
        }
    }

    // Strip binary files if needed
    char *dontstrip = ympbuild_get_value(ymp, "dontstrip");
    bool need_strip = !dontstrip || strlen(dontstrip) == 0;
    free(dontstrip);
    if (need_strip) {
        binary_process(ymp->path);
    }

    // Generate links and metadata files for the build
    generate_links_files(ymp->path);
    generate_metadata(ymp, false);

    // Duplicate the build path string to return
    char *ret = strdup(ymp->path);

    // Cleanup: free allocated resources
    archive_unref(a);
    free(ymp->header);
    free(ymp->ctx);
    free(ymp->path);
    free(ymp);
    free(build_id);
    free(ympfile);

    // Return the path of the built binary
    return ret;
}

visible bool build_from_path(const char *path) {
    // Create the source from the specified path
    char *cache = build_source_from_path(path);

    if (cache == NULL) {
        return false;
    }
    print(_("Source created at: %s\n"), cache);
    // Build the binary from the created source
    char *build = build_binary_from_path(cache);
    free(cache);
    if (build) {
        print(_("Binary package created at: %s\n"), build);
        free(build);
        return true;
    } else {
        warning(_("Failed to create package.\n"));
    }

    return false;
}

visible char *create_package(const char *path) {
    print(_("Creating package from: %s\n"), path);
    // Get the current working directory
    char curdir[PATH_MAX];
    if (getcwd(curdir, sizeof(curdir)) == NULL) {
        perror("getcwd() error");
    }
    int rc = 0;

    // Construct the path for the metadata file and the output package
    char *metadata_file = build_string("%s/metadata.yaml", path);
    char *ret = build_string("%s/package.zip", path);
    if (!metadata_file || !ret) {
        free(metadata_file);
        free(ret);
        return NULL;
    }

    // Check if the metadata file exists
    if (!isfile(metadata_file)) {
        print(_("Metadata file not found: %s\n"), metadata_file);
        free(metadata_file);
        free(ret);
        return NULL;  // Return NULL if the file is not found
    }

    // Read the contents of the metadata file
    char *metadata_raw = readfile(metadata_file);
    if (!metadata_raw) {
        free(metadata_file);
        free(ret);
        return NULL;
    }

    // Change the current directory to the specified path
    if (chdir(path) < 0) {
        print(_("Failed to change directory to: %s\n"), path);
        free(metadata_raw);
        free(metadata_file);
        free(ret);
        return NULL;  // Return NULL if changing directory fails
    }

    // Check if the metadata is valid and contains the "ymp" area
    if (!yaml_has_area(metadata_raw, "ymp")) {
        print(_("Invalid metadata format.\n"));
        free(metadata_raw);
        free(metadata_file);
        free(ret);
        rc = chdir(curdir);
        return NULL;  // Return NULL if the metadata is invalid
    }

    // Get the "ymp" area from the metadata
    char *metadata = yaml_get_area(metadata_raw, "ymp");
    free(metadata_raw);
    if (!metadata) {
        free(metadata_file);
        free(ret);
        rc = chdir(curdir);
        return NULL;
    }

    // If the "source" area exists in the metadata, create a package
    if (yaml_has_area(metadata, "source")) {
        Archive *a = archive_new();         // Create a new archive object
        if (!a || !archive_load(a, ret)) {  // Load the package file
            if (a) {
                archive_unref(a);
            }
            free(metadata);
            free(metadata_file);
            free(ret);
            rc = chdir(curdir);
            return NULL;
        }
        archive_set_type(a, "zip", "none");  // Set the archive type to ZIP

        // Find all files in the specified path
        char **files = find(path);
        for (size_t i = 0; files && files[i]; i++) {
            // Add each file to the archive, adjusting the path
            archive_add(a, files[i] + strlen(path) + 1);
        }

        // Create the archive
        archive_create(a);

        // Free the archive object and the list of files
        archive_unref(a);
        if (files) {
            for (size_t i = 0; files[i]; i++) {
                free(files[i]);
            }
            free(files);
        }
        free(metadata);
    } else if (yaml_has_area(metadata, "package")) {
        // Create a new archive object for packaging files
        Archive *a = archive_new();

        // Load the specified TAR.GZ package file into the archive object
        char *datafile = build_string("%s/data.tar.gz", path);
        if (!a || !datafile || !archive_load(a, datafile)) {
            if (a) {
                archive_unref(a);
            }
            free(datafile);
            free(metadata);
            free(metadata_file);
            free(ret);
            rc = chdir(curdir);
            return NULL;
        }

        // Set the archive type to TAR with GZIP compression
        archive_set_type(a, "tar", "gzip");

        // Change the current working directory to the 'output' directory
        if (chdir("output") < 0) {
            print(_("Failed to change directory to 'output' directory.\n"));
            archive_unref(a);
            free(datafile);
            free(metadata);
            free(metadata_file);
            free(ret);
            rc = chdir(curdir);
            return NULL;  // Return NULL if changing the directory fails
        }

        // Retrieve a list of all files in the specified path
        char **files = find(".");

        // Iterate through the list of files and add each one to the archive
        for (size_t i = 0; files && files[i]; i++) {
            // Add each file to the archive, adjusting the path to exclude the base directory
            info(_("Archive add: %s \n"), files[i]);
            archive_add(a, files[i] + 2);
        }

        // Create the archive with the added files
        archive_create(a);

        // Free the memory
        if (files) {
            for (size_t i = 0; files[i]; i++) {
                free(files[i]);
            }
            free(files);
        }
        archive_unref(a);

        // Add archive hash to metadata.yaml file
        char *hash = calculate_hash(SHA1, datafile);
        if (hash) {
            FILE *meta_fp = fopen(metadata_file, "a");
            if (meta_fp) {
                fprintf(meta_fp, "    archive-hash: %s\n", hash);  // Append  archive hash
                fflush(meta_fp);                                   // Flush file
                fclose(meta_fp);                                   // Close file
            }
            free(hash);  // Free hash after use
        }

        // Change the current working directory back to the original specified path
        if (chdir(path) < 0) {
            print(_("Failed to change directory back to: %s\n"), path);
            free(datafile);
            free(metadata);
            free(metadata_file);
            free(ret);
            rc = chdir(curdir);
            return NULL;  // Return NULL if changing the directory fails
        }

        // Create a new archive object for the final package
        a = archive_new();

        // Load the previously created package file into the new archive object
        if (!a || !archive_load(a, ret)) {
            if (a) {
                archive_unref(a);
            }
            free(datafile);
            free(metadata);
            free(metadata_file);
            free(ret);
            rc = chdir(curdir);
            return NULL;
        }

        // Set the archive type to ZIP with no compression
        archive_set_type(a, "zip", "none");

        // Add necessary files to the ZIP archive
        archive_add(a, "metadata.yaml");  // Add metadata file
        archive_add(a, "files");          // Add directory containing files
        archive_add(a, "links");          // Add directory containing links
        archive_add(a, "data.tar.gz");    // Add the previously created TAR.GZ file

        // Create the final ZIP archive with the added files
        archive_create(a);

        // Free the archive object after use
        archive_unref(a);

        // Free datafile string after use
        free(datafile);
    }

    // Free the metadata file path string
    free(metadata);
    free(metadata_file);

    // Change back to the original directory
    if (chdir(curdir) < 0) {
        print(_("Failed to change directory back.\n"));
        free(ret);
        return NULL;  // Return NULL if changing back fails
    }
    if (rc < 0) {
        return NULL;
    }

    // Return the path of the created package
    return ret;
}
