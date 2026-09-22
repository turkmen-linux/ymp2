#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utils/array.h>
#include <utils/file.h>
#include <utils/jobs.h>
#include <utils/process.h>
#include <utils/string.h>

static int pkgconf_callback(void *args) {
    char *name = (char *) args;
    pid_t pid = fork();
    if (pid == 0) {
        const FILE *fp = freopen("/dev/null", "w", stdout);
        if (fp == NULL) {
            perror("freopen");
            exit(1);
        }
        char *cmd[] = { which("pkgconf"), name, "--libs", "--cflags", NULL };
        exit(run_args(cmd));
    }
    int status;
    waitpid(pid, &status, 0);
    return status;
}

static int pkgconf_check() {
    char *dirs[] = {
        "/usr/lib/pkgconfig",
        "/lib/pkgconfig",
        "/usr/share/pkgconfig",
        NULL
    };
    for (size_t i = 0; dirs[i]; i++) {
        char **files = listdir(dirs[i]);
        jobs *job = jobs_new();
        for (size_t j = 0; files[j]; j++) {
            if (files[j][0] == '.') {
                continue;
            }
            files[j][strlen(files[j]) - 3] = '\0';
            jobs_add(job, (callback) pkgconf_callback, files[j], NULL);
        }
        jobs_run(job);
        jobs_unref(job);
        for (size_t j = 0; files[j]; j++) {
            free(files[j]);
        }
        free(files);
    }
    return 0;
}

static array *installed_libs;
static pthread_mutex_t revdep_lock = PTHREAD_MUTEX_INITIALIZER;

static int lib_compare(const void *a, const void *b) {
    return strcmp(*(const char *const *) a, *(const char *const *) b);
}

static void sort_strings(array *arr) {
    if (arr->size > 1) {
        qsort(arr->data, arr->size, sizeof(char *), lib_compare);
    }
}

static void libs_add_dir(const char *dir) {
    debug("collect libraries: %s\n", dir);
    char **files = find(dir);
    for (size_t i = 0; files[i]; i++) {
        char *base = strrchr(files[i], '/');
        array_add(installed_libs, base ? base + 1 : files[i]);
        free(files[i]);
    }
    free(files);
}

static void libs_build(void) {
    char *dirs[] = {
        "/lib",
        "/lib64",
        "/usr/lib",
        "/usr/lib64",
        "/usr/local/lib",
        "/usr/local/lib64",
        "/usr/libexec",
        NULL
    };
    installed_libs = array_new();
    for (size_t i = 0; dirs[i]; i++) {
        if (isdir(dirs[i]) && !issymlink(dirs[i])) {
            libs_add_dir(dirs[i]);
        }
    }
    array_uniq(installed_libs);
    sort_strings(installed_libs);
}

static bool lib_installed(const char *lib) {
    if (installed_libs == NULL) {
        return true;
    }
    return bsearch(&lib, installed_libs->data, installed_libs->size,
                   sizeof(char *), lib_compare) != NULL;
}

static int readelf_callback(void *args) {
    char *file = (char *) args;
    const char *cmd[] = { "readelf", "-d", file, NULL };
    char *output = getoutput_unshare((char **) cmd, 0);
    if (output == NULL) {
        return 0;
    }
    array *missing = NULL;
    char **lines = split(output, "\n");
    for (size_t i = 0; lines[i]; i++) {
        if (strstr(lines[i], "NEEDED")) {
            size_t cur = 0;
            bool found = false;
            for (size_t j = 0; lines[i][j]; j++) {
                if (lines[i][j] == '[') {
                    cur = j + 1;
                    found = true;
                } else if (lines[i][j] == ']') {
                    lines[i][j] = '\0';
                    break;
                }
            }
            if (found && !lib_installed(lines[i] + cur)) {
                if (missing == NULL) {
                    missing = array_new();
                }
                array_add(missing, lines[i] + cur);
            }
        }
        free(lines[i]);
    }
    info("Checking file: %s\n", file);
    free(output);
    free(lines);
    if (missing != NULL) {
        pthread_mutex_lock(&revdep_lock);
        size_t len;
        char **list = array_get(missing, &len);
        for (size_t i = 0; i < len; i++) {
            warning(_("Missing library: %s from %s\n"), list[i], file);
            free(list[i]);
        }
        free(list);
        pthread_mutex_unlock(&revdep_lock);
        array_unref(missing);
    }
    return 0;
}

static int readelf_check() {
    libs_build();
    char *dirs[] = {
        "/usr/bin/",
        "/usr/lib/",
        "/bin/",
        "/lib/",
        NULL
    };
    for (size_t i = 0; dirs[i]; i++) {
        char **files = find(dirs[i]);
        jobs *job = jobs_new();
        for (size_t j = 0; files[j]; j++) {
            if (files[j][0] == '.') {
                continue;
            }
            if (issymlink(files[j])) {
                continue;
            }
            if (!is_elf(files[j])) {
                continue;
            }
            jobs_add(job, (callback) readelf_callback, files[j], NULL);
        }
        jobs_run(job);
        jobs_unref(job);
        for (size_t j = 0; files[j]; j++) {
            free(files[j]);
        }
        free(files);
    }
    array_unref(installed_libs);
    installed_libs = NULL;
    return 0;
}

static int revdep_main(void **args) {
    (void) args;
    if (get_bool("pkgconfig")) {
        pkgconf_check();
    } else {
        readelf_check();
    }
    return 0;
}

void revdep_init(OperationManager *manager) {
    Operation op;
    op.name = "revdep-rebuild";
    op.alias = "rbd:cr";
    op.description = _("Check library for broken links.");
    op.min_args = 0;
    op.help = help_new();
    help_add_parameter(op.help, "--pkgconfig", _("check pkgconfig files instead of ELF binaries"));
    op.call = (callback) revdep_main;
    operation_register(manager, op);
}