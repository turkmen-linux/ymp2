#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For system calls write, read e close
#include <fcntl.h>
#include <string.h>
#include <openssl/evp.h>
#include <utils/file.h>
#define BUFFER_SIZE 1024
#define OPENSSL_API_COMPAT

visible char *calculate_sha1(const char *path) {
    unsigned char buffer[BUFFER_SIZE];
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    char hashstring[EVP_MAX_MD_SIZE*2+1];

    // https://pragmaticjoe.gitlab.io/posts/2015-02-09-how-to-generate-a-sha1-hash-in-c
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    md = EVP_sha1();
    mdctx = EVP_MD_CTX_create();

    ssize_t byte = 0;

    int fd = open(path, O_RDONLY);
    EVP_DigestInit_ex(mdctx, md, NULL);

    while ((byte = read(fd, buffer, sizeof(buffer))) != 0) {
        EVP_DigestUpdate(mdctx, buffer, byte);
        memset(buffer, 0, BUFFER_SIZE);
    }

    EVP_DigestFinal_ex(mdctx, digest, &md_len);
    EVP_MD_CTX_destroy(mdctx);
    close(fd);
    EVP_cleanup();
    for(unsigned int i = 0; i < md_len; i++){
        sprintf(&hashstring[i*2], "%02x", (unsigned int)digest[i]);
    }

    return strdup(hashstring);
}