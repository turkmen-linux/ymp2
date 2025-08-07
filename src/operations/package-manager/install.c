#include <stdio.h>

#include <core/ymp.h>
#include <core/variable.h>

#include <data/repository.h>
#include <data/dependency.h>

#include <config.h>

#include <utils/yaml.h>
#include <utils/string.h>
#include <utils/file.h>
#include <utils/jobs.h>

static int download_cb(void* args){
    Package *p = args;
    printf("Downloading: %s", p->name);
    Repository *r = (Repository*)p->repo;
    if(!package_download(p, r->uri)){
        printf("Download failed: %s", p->name);
        return 1;
    }
    return 0;
}

static int install_cb(void* args){
    Package *p = args;
    printf("Installing: %s", p->name);
    if(!package_extract(p)){
        printf("Install failed: %s", p->name);
        return 1;
    }
    return 0;
}

static int install_main(char** args){
    // Begin resolver and init job manager
    Repository **repos = resolve_begin();
    jobs* download_jobs = jobs_new();
    jobs* install_jobs = jobs_new();

    // Resolve dependencies
    Package** res = resolve_dependency(args[0]);
    // Define jobs
    for(size_t i=0; res[i];i++){
        if(package_is_installed(res[i])){
            continue;
        }
        jobs_add(download_jobs, (callback)download_cb, res[i], NULL);
        jobs_add(install_jobs, (callback)install_cb, res[i], NULL);
    }
    // Download packages
    jobs_run(download_jobs);
    jobs_run(install_jobs);

    // Cleanup resolver and job managers
    resolve_end(repos);
    jobs_unref(download_jobs);
    jobs_unref(install_jobs);
    return 0;
}

void install_init(OperationManager* manager){
    Operation op;
    op.name = "install";
    op.call = (callback)install_main;
    operation_register(manager, op);
}
