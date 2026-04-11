#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

#include <core/ymp.h>
#include <core/operations.h>

#include <utils/fetcher.h>
#include <utils/gui.h>

static VariableManager* vars;

static void fetch_progress_cb(const char* url, size_t downloaded, size_t total, void* userdata) {
    (void)url;
    char* id = (char*)userdata;

    gui_progress_update(id, downloaded, total);
    gui_progress_draw();
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

    for(size_t i=0; links[i]; i++){
        char id[32];
        snprintf(id, sizeof(id), "fetch_%zu", i);

        char title[PATH_MAX];
        snprintf(title, PATH_MAX, "%s : %s", _("Download"), basename(links[i]));

        gui_progress_add(id, title, basename(links[i]), 0);

    }
    for(size_t i=0; links[i]; i++){
        char target_file[PATH_MAX+strlen(links[i])+1];
        strcpy(target_file, target);
        strcat(target_file, "/");
        strcat(target_file, basename(links[i]));

        char id[32];
        snprintf(id, sizeof(id), "fetch_%zu", i);

        fetch_with_progress(links[i], target_file, fetch_progress_cb, id);

        gui_progress_remove(id);
    }

    gui_end();
    return 0;

}

visible void plugin_init(Ymp* ymp){
    vars = ymp->variables;
    Operation op;
    op.name = "fetch";
    op.alias = "dl:download:wget";
    op.help = NULL;
    op.min_args = 1;
    op.call = (callback) fetch_fn;
    operation_register(ymp->manager, op);
}
