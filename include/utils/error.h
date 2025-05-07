/**
 * @file error.h
 * @brief Error handling functions for managing error states and messages.
 */

#include <stdbool.h>
#include <utils/array.h>

/**
 * @brief Reports an error with a specific status.
 *
 * This function sets the error status in the provided error array.
 *
 * @param error Pointer to an array that holds error information.
 * @param status The error status code to be reported.
 */
void error(array* error, int status);

/**
 * @brief Adds an error message to the error array.
 *
 * This function appends a message to the error array, which can be used
 * for logging or debugging purposes.
 *
 * @param error Pointer to an array that holds error information.
 * @param message The error message to be added to the error array.
 */
void error_add(array* error, char* message);

/**
 * @brief Checks if there are any errors in the error array.
 *
 * This function checks the error array to determine if any errors have
 * been recorded.
 *
 * @param error Pointer to an array that holds error information.
 * @return true if there are errors in the array, false otherwise.
 */
bool has_error(array* error);

