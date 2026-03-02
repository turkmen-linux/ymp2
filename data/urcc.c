#include <stdio.h>
static char c;
static size_t i = 0;
void main(int argc, char **argv){
    FILE *f = fopen(argv[1], "rb");
    if(f == NULL){
        return;
    }
    printf("/* %s => %s\n */", argv[1], argv[2]);
    printf("unsigned char %s[] = {\n    ", argv[2]);
    while((c = getc(f)) != EOF){
        printf("%d, ", c);
        i++;
        if(i % 20 == 0){
            printf("\n    ");
        }
    }
    printf("0\n};\n");
    printf("size_t %s_size = %ld;\n\n", argv[2], i);
    fclose(f);
}
