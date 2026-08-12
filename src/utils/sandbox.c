#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/logger.h>
#include <core/variable.h>
#include <sys/mount.h>
#include <utils/file.h>
#include <utils/sandbox.h>
#include <utils/string.h>

// Writes "0 id 1" into an id map file.
static void write_id_map(const char *path, unsigned long id) {
    FILE *fp = fopen(path, "w");
    if (fp) {
        fprintf(fp, "0 %lu 1", id);
        fclose(fp);
    }
}

visible sandbox_handle_t *sandbox_new() {
    sandbox_handle_t *sandbox = calloc(1, sizeof(sandbox_handle_t));
    if (!sandbox) {
        return NULL;
    }
    sandbox->flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNET;
    sandbox->hostname = strdup("sandbox");
    sandbox->uid = getuid();
    sandbox->gid = getgid();
    sandbox->binds = array_new();
    return sandbox;
}

visible void sandbox_configure_hostname(sandbox_handle_t *sandbox, const char *hostname) {
    free(sandbox->hostname);
    sandbox->hostname = strdup(hostname);
}

visible void sandbox_configure_flags(sandbox_handle_t *sandbox, int flags) {
    sandbox->flags = flags;
}

visible void sandbox_configure_user(sandbox_handle_t *sandbox, uid_t uid, gid_t gid) {
    sandbox->uid = uid;
    sandbox->gid = gid;
}

visible void sandbox_apply(sandbox_handle_t *sandbox) {
    // Create the namespaces. Network access shares the host network.
    int flags = sandbox->network ? sandbox->flags & ~CLONE_NEWNET : sandbox->flags;
    int rc = 0;
    if (unshare(flags) < 0) {
        perror("unshare");
        exit(1);
    }
    // Isolate the mount namespace so mounts do not leak to the host.
    if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) < 0) {
        perror("mount");
        exit(1);
    }
    // Isolate the hostname.
    if (sethostname(sandbox->hostname, strlen(sandbox->hostname)) < 0) {
        perror("hostname");
        exit(1);
    }
    // Map the current user to root inside the sandbox.
    FILE *setgroups = fopen("/proc/self/setgroups", "w");
    if (setgroups) {
        fprintf(setgroups, "deny");
        fclose(setgroups);
    }
    write_id_map("/proc/self/uid_map", sandbox->uid);
    write_id_map("/proc/self/gid_map", sandbox->gid);

    // New rootfs.
    create_dir("/tmp/ymp-root");
    rc = chdir("/tmp/ymp-root");
    if (rc) {
        exit(rc);
    }

    // Apply the registered mounts inside the rootfs.
    size_t len = 0;
    char **binds = array_get(sandbox->binds, &len);
    for (size_t i = 0; i < len; i++) {
        char **parts = split(binds[i], " ");
        char *target = build_string("/tmp/ymp-root%s", parts[1]);
        // A "tmpfs" source is mounted as a fresh tmpfs instead of a bind mount.
        int ret;
        if (strcmp(parts[0], "tmpfs") == 0) {
            ret = mount("tmpfs", target, "tmpfs", 0, NULL);
        } else {
            debug("%s => %s\n", parts[0], target);
            create_dir(target);
            ret = mount(parts[0], target, NULL, MS_BIND | MS_REC, NULL);
        }
        if (ret < 0) {
            perror("mount");
            warning("Failed to mount: %s\n", target);
            continue;
        }
        free(target);
        free(binds[i]);
        free(parts);
    }
    free(binds);

    // Chroot into the rootfs.
    rc = chroot("/tmp/ymp-root");
    if (rc) {
        exit(rc);
    }
    rc = chdir("/");
    if (rc) {
        exit(rc);
    }
}

visible void sandbox_configure_bind(sandbox_handle_t *sandbox, const char *src, const char *target) {
    array_add(sandbox->binds, build_string("%s %s", src, target));
}

visible void sandbox_configure_network(sandbox_handle_t *sandbox, bool enabled) {
    sandbox->network = enabled;
}

visible void sandbox_unref(sandbox_handle_t *sandbox) {
    free(sandbox->hostname);
    array_unref(sandbox->binds);
    free(sandbox);
}

visible void sandbox() {
    if (get_bool("no-sandbox")) {
        return;
    }
    sandbox_handle_t *handle = sandbox_new();
    if (handle) {
        sandbox_apply(handle);
        sandbox_unref(handle);
    }
}
