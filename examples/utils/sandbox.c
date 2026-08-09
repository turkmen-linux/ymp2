#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <utils/sandbox.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Create and configure the sandbox.
    sandbox_handle_t *handle = sandbox_new();
    if (!handle) {
        return EXIT_FAILURE;
    }
    sandbox_configure_hostname(handle, "sandbox");
    sandbox_configure_bind(handle, "tmpfs", "/tmp");
    sandbox_configure_network(handle, false);

    // Apply the sandbox and run the command inside it.
    sandbox_apply(handle);
    sandbox_unref(handle);

    execvp(argv[1], &argv[1]);
    perror("execvp");
    return EXIT_FAILURE;
}
