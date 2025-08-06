/**
 * @file error.h
 * @brief Error handling functions for managing error states and messages.
 */

#include <stdbool.h>
#include <utils/array.h>

/** @cond */
#ifndef invisible
#define invisible ;//
#endif
/** @endcond */

/**
 * @brief Reports an error with a specific status.
 *
 * This function sets the error status in the provided error array.
 *
 * @param error Pointer to an array that holds error information.
 * @param status The error status code to be reported.
 */
void error_fn(array* error, int status);
/** @cond */
invisible void error(int status);
/** @endcond */

/**
 * @brief Adds an error message to the error array.
 *
 * This function appends a message to the error array, which can be used
 * for logging or debugging purposes.
 *
 * @param error Pointer to an array that holds error information.
 * @param message The error message to be added to the error array.
 */
void error_add_fn(array* error, const char* message);
/** @cond */
invisible void error_add(const char* message);
/** @endcond */

/**
 * @brief Checks if there are any errors in the error array.
 *
 * This function checks the error array to determine if any errors have
 * been recorded.
 *
 * @param error Pointer to an array that holds error information.
 * @return true if there are errors in the array, false otherwise.
 */
bool has_error_fn(array* error);
/** @cond */
invisible bool has_error();
/** @endcond */

