#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/array.h>


#include <core/ymp.h>

int main() {
    (void)ymp_init();
    // Create a new dynamic array
    array *myArray = array_new();
    if (myArray == NULL) {
        fprintf(stderr, "Failed to create dynamic array\n");
        return EXIT_FAILURE;
    }

    // Add some strings to the dynamic array
    array_add(myArray, "apple");
    array_add(myArray, "banana");
    array_add(myArray, "orange");
    array_add(myArray, "banana"); // Adding a duplicate for testing
    array_add(myArray, "grape");

    // Get the current contents of the array
    size_t len;
    char **contents = array_get(myArray, &len);
    printf("Current contents of the array:\n");
    for (size_t i = 0; i < len; i++) {
        printf("%s\n", contents[i]);
    }

    // Remove duplicates
    array_uniq(myArray);
    printf("\nContents after removing duplicates:\n");
    contents = array_get(myArray, &len);
    for (size_t i = 0; i < len; i++) {
        printf("%s\n", contents[i]);
    }

    // Sort the array
    array_sort(myArray);
    printf("\nContents after sorting:\n");
    contents = array_get(myArray, &len);
    for (size_t i = 0; i < len; i++) {
        printf("%s\n", contents[i]);
    }

    // Concatenate all strings into a single string
    char *concatenated = array_get_string(myArray);
    if (concatenated) {
        printf("\nConcatenated string:\n%s\n", concatenated);
        free(concatenated); // Free the concatenated string
    }

    // Clean up and free the dynamic array
    array_unref(myArray);

    return 0;
}
