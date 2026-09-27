#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/archive.h>
#include <utils/file.h>

visible bool deb_extract(const char *package, const char *target) {
    // 1. extract deb package
    Archive *deb = archive_new();
    if (!archive_load(deb, package)) {
        archive_unref(deb);
        return false;
    }
    archive_set_target(deb, target);
    archive_extract_all(deb);
    archive_unref(deb);
    // 2. extract package files
    char **files = listdir(target);
    char path[PATH_MAX + 1];
    char path2[PATH_MAX + 8];
    for (size_t j = 0; files[j]; j++) {
        snprintf(path, sizeof(path), "%s/%s", target, files[j]);
        if (strncmp(files[j], "control.tar.", 12) == 0) {
            Archive *control = archive_new();
            if (!archive_load(control, path)) {
                archive_unref(control);
                for (size_t k = 0; files[k]; k++) {
                    free(files[k]);
                }
                free(files);
                return false;
            }
            snprintf(path2, sizeof(path2), "%s/DEBIAN", target);
            archive_set_target(control, path2);
            archive_extract_all(control);
            archive_unref(control);
            unlink(path);
        } else if (strncmp(files[j], "data.tar.", 9) == 0) {
            Archive *data = archive_new();
            if (!archive_load(data, path)) {
                archive_unref(data);
                for (size_t k = 0; files[k]; k++) {
                    free(files[k]);
                }
                free(files);
                return false;
            }
            archive_set_target(data, target);
            data->preserve_perm = true;
            archive_extract_all(data);
            archive_unref(data);
        }
        unlink(path);
    }
    for (size_t k = 0; files[k]; k++) {
        free(files[k]);
    }
    free(files);
    return true;
}
