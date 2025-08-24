#include <stdio.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <core/variable.h>

#include <data/repository.h>
#include <data/dependency.h>
#include <data/quarantine.h>

#include <config.h>

#include <utils/yaml.h>
#include <utils/string.h>
#include <utils/file.h>
#include <utils/jobs.h>

typedef struct {
    int cur;
    int total;
} proc;

static int download_cb(Package* p, int num){
    debug("download %d", num);
    print("%s: %s\n", colorize(YELLOW, "Downloading"), p->name);
    Repository *r = (Repository*)p->repo;
    if(!package_download(p, r->uri)){
        print("%s: %s\n", colorize(RED, "Download Failed"), p->name);
        return 1;
    }
    package_load_from_file(p, p->path);
    return 0;
}

static int install_cb(Package*p, int num){
    debug("install %d", num);
    print("%s: %s\n", colorize(YELLOW, "Installing"), p->name);
    if(!package_extract(p)){
        print("%s: %s\n", colorize(RED, "Install Failed"), p->name);
        return 1;
    }
    return 0;
}

static int install_main(char** args){
    // Begin resolver and init job manager
    Repository **repos = resolve_begin();
    jobs* download_jobs = jobs_new();
    jobs* install_jobs = jobs_new();

    for(size_t r=0; args[r]; r++){
        // Resolve dependencies
        Package** res = resolve_dependency(args[0]);
        // Define jobs
        for(size_t i=0; res[i];i++){
            if(package_is_installed(res[i])){
                continue;
            }
            jobs_add(download_jobs, (callback)download_cb, res[i], (void*)i+1);
            jobs_add(install_jobs, (callback)install_cb, res[i], (void*)i+1);
        }
    }
    // Download packages
    jobs_run(download_jobs);
    jobs_run(install_jobs);

    // Quarantine validate and sync
    quarantine_validate();

    // Cleanup resolver and job managers
    resolve_end(repos);
    jobs_unref(download_jobs);
    jobs_unref(install_jobs);
    return 0;
}

void install_init(OperationManager* manager){
    Operation op;
    op.name = "install";
    op.alias = "it:add:merge";
    op.min_args = 1;
    op.call = (callback)install_main;
    operation_register(manager, op);
}
