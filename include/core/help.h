/**
 * @file help.h
 * @brief Header file for Help structure and related functions.
 */

#ifndef HELP_H
#define HELP_H

#include <stddef.h>

/**
 * @struct Help
 * @brief A structure to manage help information for a command-line application.
 *
 * This structure holds parameters, usage information, and keeps track of the current
 * and maximum number of parameters.
 */
typedef struct {
    char** parameters; /**< Array of parameter strings. */
    const char* usage; /**< Usage information string. */
    size_t cur;        /**< Current number of parameters added. */
    size_t max;       /**< Maximum number of parameters that can be stored. */
} Help;

/**
 * @brief Creates a new Help structure.
 *
 * Allocates memory for a new Help structure and initializes its members.
 *
 * @return A pointer to the newly created Help structure, or NULL if allocation fails.
 */
Help* help_new();

void help_unref(Help* h);

/**
 * @brief Adds a string to the Help structure.
 *
 * This function appends a string to the parameters array in the Help structure.
 *
 * @param h A pointer to the Help structure.
 * @param string The string to be added to the parameters.
 */
void help_add_string(Help* h, const char* string);

/**
 * @brief Adds a parameter with a description to the Help structure.
 *
 * This function adds a parameter name and its description to the Help structure.
 *
 * @param h A pointer to the Help structure.
 * @param name The name of the parameter to be added.
 * @param description The description of the parameter.
 */
void help_add_parameter(Help* h, const char* name, const char* description);

#endif // HELP_H
