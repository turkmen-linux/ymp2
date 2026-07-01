#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <core/ymp.h>
#include <core/operations.h>

#include <utils/fetcher.h>
#include <utils/gui.h>
#include <utils/jobs.h>

static VariableManager* vars;

static void fetch_progress_cb(const char* url, size_t downloaded, size_t total, void* userdata) {
    (void)url;
    char* id = (char*)userdata;

    gui_progress_update(id, downloaded, total);
}

typedef struct {
    const char* url;
    char* target_file;
    char* id;
} download_job_t;

static int download_job_cb(void* ctx, void* args) {
    (void)args;
    download_job_t* dl = (download_job_t*)ctx;

    char title[PATH_MAX];
    const char* base = strrchr(dl->url, '/');
    base = base ? base + 1 : dl->url;
    snprintf(title, PATH_MAX, "%s : %s", _("Download"), base);
    gui_progress_add(dl->id, title, base, 0);

    bool ok = fetch_with_progress(dl->url, dl->target_file, fetch_progress_cb, dl->id);
    gui_progress_remove(dl->id);
    free(dl->target_file);
    free(dl->id);
    free(dl);
    return ok ? 0 : 1;
}

static int fetch_fn(void** args){
    const char* target = variable_get_value(vars, "target");
    char **links = (char**)args;

    char curdir[PATH_MAX];
    if(!target){
        if (getcwd(curdir, sizeof(curdir)) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        target = curdir;
    }

    gui_init();

    jobs* j = jobs_new();
    if (!j) {
        gui_end();
        return 1;
    }

    for(size_t i=0; links[i]; i++){
        download_job_t* dl = malloc(sizeof(download_job_t));
        dl->url = links[i];

        const char* base = strrchr(links[i], '/');
        base = base ? base + 1 : links[i];

        char target_file[PATH_MAX + strlen(links[i]) + 1];
        snprintf(target_file, sizeof(target_file), "%s/%s", target, base);
        dl->target_file = strdup(target_file);

        char id[32];
        snprintf(id, sizeof(id), "fetch_%zu", i);
        dl->id = strdup(id);

        jobs_add(j, (callback)download_job_cb, dl, NULL);
    }

    jobs_run(j);
    jobs_unref(j);

    gui_end();
    return 0;

}

visible void plugin_init(Ymp* ymp){
    vars = ymp->variables;
    Operation op;
    op.name = "fetch";
    op.alias = "dl:download:wget";
    op.description = _("file downloader");
    op.help = help_new();
    help_add_parameter(op.help, "--target", _("Target directory"));
    op.min_args = 1;
    op.call = (callback) fetch_fn;
    operation_register(ymp->manager, op);
}
