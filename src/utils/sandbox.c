#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <core/variable.h>
#include <utils/sandbox.h>

visible void sandbox() {
    if (get_bool("no-sandbox")) {
        return;
    }
    uid_t uid = getuid();
    gid_t gid = getgid();
    if (unshare(UNSHARE_FLAGS) < 0) {
        perror("unshare");
        exit(1);
    }
    if (sethostname("sandbox", 7) < 0) {
        perror("hostname");
        exit(1);
    }
    FILE *uid_map = fopen("/proc/self/uid_map", "w");
    if (uid_map) {
        fprintf(uid_map, "0 %d 1", uid);
        fclose(uid_map);
    }
    FILE *setgroups = fopen("/proc/self/setgroups", "w");
    if (setgroups) {
        fprintf(setgroups, "deny");
        fclose(setgroups);
    }
    FILE *gid_map = fopen("/proc/self/gid_map", "w");
    if (gid_map) {
        fprintf(gid_map, "0 %d 1", gid);
        fclose(gid_map);
    }
}
