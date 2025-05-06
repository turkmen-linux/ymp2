#include <stdio.h>
#include <ymp.h>

int main(int argc, char** argv){
    (void) argc; (void)argv;
    Ymp* ymp = ymp_init(); // Create ymp context
    printf("List all operations:\n");
    for(size_t i=0; i< ymp->manager->length;i++){
        char* name = ymp->manager->operations[i].name; // Fetch operation name from manager
        printf("  Operation: %s\n", name); // Print operation name
    }
    int rc = operation_main(ymp->manager, "print", "Hello World\n"); // Run an operation
    ymp_add(ymp, "print", "Hello\n");
    ymp_add(ymp, "print", "World\n");
    rc = ymp_run(ymp);
    return rc;
}