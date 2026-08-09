#include "utils/gpg.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Check for the correct number of arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <file_to_sign> <export_path> <signed_file> <keyring_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *file_to_sign = argv[1];
    const char *export_path = argv[2];
    const char *signed_file = argv[3];
    const char *keyring_path = argv[4];
    const char *repicent = argv[5];

    set_gpg_repicent((char *) repicent);

    // Sign the file
    if (gpg_sign_file(file_to_sign)) {
        printf("Successfully signed the file: %s\n", file_to_sign);
    } else {
        printf("Failed to sign the file: %s\n", file_to_sign);
    }

    // Export the public key
    if (gpg_export_file(export_path)) {
        printf("Successfully exported the public key to: %s\n", export_path);
    } else {
        printf("Failed to export the public key to: %s\n", export_path);
    }

    // Verify the signed file
    if (verify_file(signed_file, keyring_path)) {
        printf("The signature of the file %s is valid.\n", signed_file);
    } else {
        printf("The signature of the file %s is invalid or verification failed.\n", signed_file);
    }

    return EXIT_SUCCESS;
}
