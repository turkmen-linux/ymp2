#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/logger.h>
#include <core/operations.h>
#include <core/ymp.h>
#include <utils/sandbox.h>
#include <utils/string.h>

static VariableManager *vars;

static int sandbox_fn(char **args) {
    if (!args[0]) {
        print(_("No command to run in the sandbox.\n"));
        return 1;
    }
    sandbox_handle_t *handle = sandbox_new();
    if (!handle) {
        return 1;
    }

    char *hostname = variable_get_value(vars, "hostname");
    if (hostname && strlen(hostname)) {
        sandbox_configure_hostname(handle, hostname);
    }
    char *bind = variable_get_value(vars, "bind");
    if (bind && strlen(bind)) {
        char **mounts = split(bind, " ");
        for (size_t i = 0; mounts[i]; i++) {
            char **parts = split(mounts[i], ":");
            sandbox_configure_bind(handle, parts[0], parts[1]);
        }
    }
    char *network = variable_get_value(vars, "network");
    if (network && strcmp(network, "true") == 0) {
        sandbox_configure_network(handle, true);
    }

    sandbox_apply(handle);
    sandbox_unref(handle);

    execvp(args[0], args);
    perror("execvp");
    return 1;
}

visible void plugin_init(Ymp *ymp) {
    vars = ymp->variables;
    Operation op;
    op.name = "sandbox";
    op.description = _("Run a command inside an isolated sandbox");
    op.alias = NULL;
    op.help = help_new();
    help_add_parameter(op.help, "--hostname", _("Set the sandbox hostname."));
    help_add_parameter(op.help, "--bind", _("Bind mounts (space separated src:target) into the sandbox."));
    help_add_parameter(op.help, "--network", _("Share the host network."));
    op.min_args = 1;
    op.call = (callback) sandbox_fn;
    operation_register(ymp->manager, op);
}
