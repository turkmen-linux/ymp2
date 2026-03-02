#include <stdio.h>
void main(int argc, char **argv){
    FILE *f = fopen(argv[1], "rb");
    char c;
    size_t i = 0;
    printf("unsigned char %s[] = {", argv[2]);
    while((c = getc(f)) != EOF){
        printf("%d, ", c);
        i++;
    }
    printf("0};\n");
    printf("size_t %s_size = %ld;\n", argv[2], i);
}
