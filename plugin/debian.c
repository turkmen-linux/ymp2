#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/logger.h>
#include <core/operations.h>
#include <core/ymp.h>
#include <utils/archive.h>
#include <utils/debian.h>
#include <utils/file.h>

static VariableManager *vars;

static int deb_fn(char **args) {
    char *target = variable_get_value(vars, "target");
    char curdir[PATH_MAX];
    if (!target || strlen(target) == 0) {
        if (getcwd(curdir, sizeof(curdir)) == NULL) {
            perror("getcwd() error");
            return 1;
        }
        target = curdir;
    }
    int status = 0;
    for (size_t i = 0; args[i]; i++) {
        if (variable_get_value(vars, "extract")) {
            if (!deb_extract(args[i], target)) {
                status = 1;
                break;
            }
        }
    }
    return status;
}

visible void plugin_init(Ymp *ymp) {
    vars = ymp->variables;
    Operation op;
    op.name = "debian";
    op.description = _("Deb package operations");
    op.alias = "deb";
    op.help = help_new();
    help_add_parameter(op.help, "--extract", _("Extract a deb package."));
    help_add_parameter(op.help, "--target", _("Extract directory target."));
    op.min_args = 1;
    op.call = (callback) deb_fn;
    operation_register(ymp->manager, op);
}
