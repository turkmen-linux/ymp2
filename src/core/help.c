#include <stdlib.h>
#include <stdio.h>

#include <core/help.h>
#include <core/logger.h>

#include <utils/string.h>

// Function to create a new Help structure
visible Help* help_new() {
    // Allocate memory for the Help structure
    Help *h = malloc(sizeof(Help));
    if (h == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed for Help structure.\n");
        return NULL;
    }

    // Initialize current parameter count and maximum capacity
    h->cur = 0;
    h->max = 32;

    // Allocate memory for the parameters array
    h->parameters = malloc(h->max * sizeof(char*));
    if (h->parameters == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed for parameters array.\n");
        free(h); // Free previously allocated memory for Help structure
        return NULL;
    }

    return h; // Return the newly created Help structure
}

// Function to add a string to the Help structure
visible void help_add_string(Help *h, const char* string) {
    // Check if the current count has reached the maximum capacity
    if (h->cur >= h->max) {
        // Increase the maximum capacity by 32
        h->max += 32;
        // Reallocate memory for the parameters array
        h->parameters = realloc(h->parameters, h->max * sizeof(char*));
        if (h->parameters == NULL) {
            // Handle memory reallocation failure
            fprintf(stderr, "Memory reallocation failed for parameters array.\n");
            return; // Exit the function if reallocation fails
        }
    }

    // Duplicate the string and store it in the parameters array
    h->parameters[h->cur] = strdup(string);
    if (h->parameters[h->cur] == NULL) {
        // Handle string duplication failure
        fprintf(stderr, "String duplication failed for: %s\n", string);
        return; // Exit the function if duplication fails
    }

    // Increment the current parameter count
    h->cur++;
}

// Function to add a parameter with its description to the Help structure
visible void help_add_parameter(Help *h, const char* name, const char* description) {
    // Log the name and description for debugging purposes
    debug("name: %s, desc: %s\n", name, description);

    // Build a formatted string combining the name and description, and add it to the Help structure
    char* string = build_string(colorize_fn(colorized(RED, "%s")" : %s", "%s : %s"), name, description);
    help_add_string(h, (const char*)string);
    free(string);
}
