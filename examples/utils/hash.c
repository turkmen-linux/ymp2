#include <stdio.h>
#include <utils/hash.h>

int main(){
    char* path = "/etc/os-release";
    char* sha1 = calculate_sha1(path);
    printf("SHA1 %s\n", sha1);
    return 0;
}