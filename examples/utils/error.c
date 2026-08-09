#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <core/ymp.h>
#include <utils/array.h>
#include <utils/error.h>

int main() {

    // Create an error array
    array *error_array = array_new();  // Assuming array_create initializes an empty array

    // Simulate error handling
    error_add_fn(error_array, "Failed to complete some_operation.");
    error_add_fn(error_array, "Failed to complete another_operation.");

    // Check if there are any errors
    if (has_error_fn(error_array)) {
        // Report errors and exit with a status code
        error_fn(error_array, 0);
    }

    // Clean up
    array_unref(error_array);  // Assuming array_destroy frees the allocated memory for the array

    return 1;
}
