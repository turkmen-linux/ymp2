#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <core/logger.h>
#include <core/variable.h>
#include <core/ymp.h>
#include <data/dependency.h>
#include <data/quarantine.h>
#include <data/repository.h>
#include <utils/file.h>
#include <utils/jobs.h>
#include <utils/string.h>
#include <utils/yaml.h>

static int install_upgrade();

static bool in_upgrade = false;

static int download_cb(Package *p, int num) {
    print("%s: %s\n", "Downloading", p->name);
    Repository *r = (Repository *) p->repo;
    debug("download %d %s\n", num, r->uri);
    if (!package_download(p, r->uri)) {
        print("%s: %s\n", "Download Failed", p->name);
        return 1;
    }
    package_load_from_file(p, p->path);
    return 0;
}

static int install_cb(Package *p, int num) {
    debug("install %d", num);
    print("%s: %s\n", "Installing", p->name);
    if (!package_extract(p)) {
        print("%s: %s\n", "Install Failed", p->name);
        return 1;
    }
    if (get_bool("sync-single")) {
        return quarantine_validate();
    }
    return 0;
}

static int install_main(char **args) {
    int status = 0;
    if (get_bool("upgrade") && !in_upgrade) {
        status = install_upgrade();
        if (status != 0) {
            return status;
        }
    }

    // Begin resolver and init job manager
    Repository **repos = resolve_begin();
    if (repos == NULL) {
        return 2;
    }
    jobs *download_jobs = jobs_new();
    jobs *install_jobs = jobs_new();
    // single thread install if sync single or source package installation
    if (get_bool("sync-single") || !get_bool("no-emerge")) {
        install_jobs->parallel = 1;
    }

    for (size_t r = 0; args[r]; r++) {
        // Resolve dependencies
        Package **res = resolve_dependency(args[0]);
        if (res == NULL) {
            continue;
        }
        // Define jobs
        for (size_t i = 0; res[i]; i++) {
            if (package_is_installed(res[i])) {
                continue;
            }
            jobs_add(download_jobs, (callback) download_cb, res[i], (void *) (i + 1));
            jobs_add(install_jobs, (callback) install_cb, res[i], (void *) (i + 1));
        }
    }
    // Download packages
    jobs_run(download_jobs);
    if (download_jobs->failed) {
        status = 1;
        goto install_main_free;
    }

    jobs_run(install_jobs);
    if (install_jobs->failed) {
        status = 1;
        goto install_main_free;
    }

    // Quarantine validate and sync
    if (!quarantine_validate()) {
        status = 1;
    }

install_main_free:

    // Cleanup resolver and job managers
    resolve_end(repos);
    jobs_unref(download_jobs);
    jobs_unref(install_jobs);
    return status;
}

static int install_upgrade() {
    // Begin resolver and init job manager
    Repository **repos = resolve_begin();
    if (repos == NULL) {
        return 2;
    }
    char **need_upgrade = resolve_upgrade(repos);
    in_upgrade = true;                        // semaphore
    int status = install_main(need_upgrade);  // first upgrade packages
    in_upgrade = false;
    // Clean up
    for (size_t i = 0; need_upgrade[i]; i++) {
        free(need_upgrade[i]);
    }
    free(need_upgrade);
    resolve_end(repos);
    return status;
}

void install_init(OperationManager *manager) {
    Operation op;
    op.name = "install";
    op.alias = "it:add:merge";
    op.description = _("Install package");
    op.min_args = 1;
    op.call = (callback) install_main;
    op.help = help_new();
    help_add_parameter(op.help, "--ignore-dependency", _("disable dependency check"));
    help_add_parameter(op.help, "--reinstall", _("reinstall if already installed"));
    help_add_parameter(op.help, "--no-emerge", _("use binary package"));
    help_add_parameter(op.help, "--sync-single", _("sync quarantine after every package installation"));
    operation_register(manager, op);
}
