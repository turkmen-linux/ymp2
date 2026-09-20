#include <config.h>
#include <stdio.h>
#include <stdlib.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <data/quarantine.h>
#include <utils/file.h>
#include <utils/string.h>

static int clean_main(char **args) {
    (void) args;
    char *destdir = get_value("DESTDIR");
    char *path = NULL;

    print(_("Clean: %s\n"), _("package cache"));
    path = build_string("%s/%s/packages", destdir, STORAGE);
    remove_all(path);
    free(path);

    print(_("Clean: %s\n"), _("repository index cache"));
    path = build_string("%s/%s/index", destdir, STORAGE);
    remove_all(path);
    free(path);

    print(_("Clean: %s\n"), _("build directory"));
    path = build_string("%s/tmp/ymp-build/", destdir);
    remove_all(path);
    free(path);

    print(_("Clean: %s\n"), _("quarantine"));
    quarantine_reset();
    return 0;
}

void clean_init(OperationManager *manager) {
    Operation op;
    op.name = "clean";
    op.alias = "cc";
    op.description = _("Remove all caches.");
    op.min_args = 0;
    op.help = help_new();
    help_add_string(op.help, _("Takes no arguments. Removes package, index and build caches."));
    op.call = (callback) clean_main;
    operation_register(manager, op);
}
