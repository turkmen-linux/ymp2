#include <stdio.h>

#include <core/ymp.h>
#include <data/package.h>
#include <utils/error.h>

int main(int argc, char **argv) {
    Ymp *ymp = ymp_init();
    // Create a package
    Package *pkg = package_new();
    pkg->name = "foo";
    pkg->version = "1.0";
    char *deps[] = { "bar", "bazz" };
    pkg->dependencies = deps;
    pkg->release = 1;
    pkg->archive = archive_new();

    Package *pkg2 = package_new();
    const char *path = "path/to/package.zip";
    if (argc > 1) {
        path = argv[1];
    }
    package_load_from_file(pkg2, path);
    error_fn(ymp->errors, 0);
    printf("%s\n", pkg2->metadata);
    variable_set_value(ymp->variables, "DESTDIR", "./");
    package_extract(pkg2);
    return 0;
}
