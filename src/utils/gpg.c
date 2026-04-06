#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

#include <core/variable.h>

#include <utils/file.h>
#include <utils/process.h>
#include <utils/gpg.h>

#include <config.h>

static char* gpg_repicent;

visible void set_gpg_repicent(char* repicent){
    gpg_repicent = repicent;
}

visible bool gpg_sign_file(const char *path) {
    if (!isfile(path)) {
        return false;
    }
    char *args[] = {"gpg", "--batch", "--yes", "--sign", "-r", gpg_repicent, (char *)path, NULL};
    return 0 == run_args(args);
}

visible bool gpg_export_file(const char *path) {
    if (isfile(path)) {
        return false;
    }
    char *args[] =  {"gpg", "--armor", "--export", "-o", gpg_repicent, (char*)path, NULL};
    return 0 == run_args(args);
}

visible bool verify_file(const char *path, const char* keyring) {
    if (get_bool("ignore-gpg")){
        return true;
    }
    if (!isfile(path)) {
        return false;
    }
    char gpgdir[PATH_MAX];
    snprintf(gpgdir, sizeof(gpgdir), "%s/%s/gpg", get_value("DESTDIR"), STORAGE);

    char sig[PATH_MAX];
    snprintf(sig, sizeof(sig), "%s.gpg", path);

    char *args[] = {"gpg", "--homedir", gpgdir, "--trust-model", "always", "--no-default-keyring", "--keyring", (char*)keyring, "--quiet", "--verify", sig, NULL};
    return 0 == run_args(args);
}

