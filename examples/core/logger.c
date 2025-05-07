#include <stdio.h>
#include <stdbool.h>

#include <core/logger.h>

int main() {
    // Set the logging status
    logger_set_status(PRINT, false);  // Disable debug logging
    logger_set_status(DEBUG, true);  // Enable debug logging
    logger_set_status(INFO, true);    // Enable info logging
    logger_set_status(WARNING, false); // Disable warning logging


    // Log a print message
    print("Printing value: %d\n", 42);

    logger_set_status(PRINT, true);  // Enable debug logging

    // Log a general print message
    print("This is a general print message.\n");

    // Log a debug message
    debug("Debugging value: %d\n", 42);

    // Log a debug message
    print_fn(PRINT,"print value: %d\n", 42);

    // Log an informational message
    info("Informational message: %s\n", "Everything is running smoothly.");

    // Log a warning message (this will not be printed since warnings are disabled)
    warning("Warning: This is a warning message that won't be shown.\n");

    // Change the logging status to enable warnings
    logger_set_status(WARNING, true);
    warning("Warning: This is a warning message that will be shown now.\n");

    return 0;
}
