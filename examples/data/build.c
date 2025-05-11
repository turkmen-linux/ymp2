#include <stdlib.h>
#include <stdio.h>

#include <data/build.h>
#include <core/logger.h>

int main(int argc, char* argv[]) {
    // Check if the user provided a path as an argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        return 0;
    }

    logger_set_status(DEBUG, true);

    const char* path = argv[1];

    // Call the build_from_path function
    if (build_from_path(path)) {
        printf("Build environment successfully created from path: %s\n", path);
    } else {
        fprintf(stderr, "Failed to create build environment from path: %s\n", path);
        return 1;
    }

    return 0;
}

