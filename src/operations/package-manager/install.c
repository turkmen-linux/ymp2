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


static int download_cb(Package* p, int num){
    print("%s: %s\n", "Downloading", p->name);
    Repository *r = (Repository*)p->repo;
    debug("download %d %s\n", num, r->uri);
    if(!package_download(p, r->uri)){
        print("%s: %s\n", "Download Failed", p->name);
        return 1;
    }
    package_load_from_file(p, p->path);
    return 0;
}

static int install_cb(Package*p, int num){
    debug("install %d", num);
    print("%s: %s\n", "Installing", p->name);
    if(!package_extract(p)){
        print("%s: %s\n", "Install Failed", p->name);
        return 1;
    }
    if(get_bool("sync-single")){
        return quarantine_validate();
    }
    return 0;
}

static int install_main(char** args){
    // Begin resolver and init job manager
    Repository **repos = resolve_begin();
    if(repos == NULL){
        return 2;
    }
    jobs* download_jobs = jobs_new();
    jobs* install_jobs = jobs_new();
    // single thread install if sync single or source package installation
    if(get_bool("sync-single") || !get_bool("no-emerge")){
        install_jobs->parallel = 1;
    }
    int status = 0;

    for(size_t r=0; args[r]; r++){
        // Resolve dependencies
        Package** res = resolve_dependency(args[0]);
        if(res == NULL){
            continue;
        }
        // Define jobs
        for(size_t i=0; res[i];i++){
            if(package_is_installed(res[i])){
                continue;
            }
            jobs_add(download_jobs, (callback)download_cb, res[i], (void*)(i+1));
            jobs_add(install_jobs, (callback)install_cb, res[i], (void*)(i+1));
        }
    }
    // Download packages
    jobs_run(download_jobs);
    if(download_jobs->failed){
        status = 1;
        goto install_main_free;
    }

    jobs_run(install_jobs);
    if(install_jobs->failed){
        status = 1;
        goto install_main_free;
    }

    // Quarantine validate and sync
    if(!quarantine_validate()){
        status = 1;
    }

install_main_free:

    // Cleanup resolver and job managers
    resolve_end(repos);
    jobs_unref(download_jobs);
    jobs_unref(install_jobs);
    return status;
}

void install_init(OperationManager* manager){
    Operation op;
    op.name = "install";
    op.alias = "it:add:merge";
    op.description = "Install package";
    op.min_args = 1;
    op.call = (callback)install_main;
    op.help = help_new();
    help_add_parameter(op.help, "--ignore-dependency", "disable dependency check");
    help_add_parameter(op.help, "--reinstall", "reinstall if already installed");
    help_add_parameter(op.help, "--no-emerge", "use binary package");
    help_add_parameter(op.help, "--sync-single", "sync quarantine after every package installation");
    operation_register(manager, op);
}
