#include <stdio.h>
#include <stdlib.h>

#include <core/logger.h>
#include <core/ymp.h>
#include <data/dependency.h>

int main(int argc, char *argv[]) {
    (void) ymp_init();
    char *pkg = "mesa";
    if (argc > 1) {
        pkg = argv[1];
    }
    Repository **repos = resolve_begin();
    Package **resolved = resolve_dependency(pkg);
    if (!resolved) {
        printf("Package not found!\n");
        return 0;
    }
    for (size_t i = 0; resolved[i]; i++) {
        printf("%s ", resolved[i]->name);
    }
    puts("");
    resolve_end(repos);
    return 0;
}
