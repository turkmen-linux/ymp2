#include <stdio.h>
#include <utils/hash.h>

int main(){
    char* path = "/etc/os-release";

    char* sha1 = calculate_sha1(path);
    printf("SHA1 %s\n", sha1);

    char* md5 = calculate_md5(path);
    printf("MD5 %s\n", md5);

    char* sha256 = calculate_sha256(path);
    printf("SHA256 %s\n", sha256);

    char* sha512 = calculate_sha512(path);
    printf("SHA512 %s\n", sha512);

    return 0;
}