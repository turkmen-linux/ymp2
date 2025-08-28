#ifndef _process_h
#define _process_h
/**
 * @file process.h
 * @brief Header file for process management functions.
 *
 * This header file contains declarations for functions that handle
 * process-related operations, including retrieving the current epoch
 * time and ensuring a single instance of the application is running.
 */

#include <stddef.h>

/**
 * @brief Get the current epoch time.
 *
 * This function retrieves the current epoch time in micro seconds.
 * The epoch time is the number of seconds that have elapsed since
 * January 1, 1970 (UTC).
 *
 * @return The current epoch time as a size_t value.
 */
size_t get_epoch();

/**
 * @brief Ensure a single instance of the application is running.
 *
 * This function checks if another instance of the application is
 * already running. If another instance is found, it may terminate
 * the current instance or take appropriate action to prevent
 * multiple instances from running simultaneously.
 */
void single_instance();

/**
 * @brief Executes a command with the given arguments.
 *
 * This function takes an array of strings representing command-line arguments
 * and executes the corresponding command. It is typically used to run external
 * programs or scripts from within a C program.
 *
 * @param args An array of character pointers (strings) representing the command
 *             and its arguments. The first element should be the command to
 *             execute, followed by its arguments. The last element must be NULL
 *             to indicate the end of the arguments.
 *
 * @return int Returns 0 on success, or a negative value on failure. The specific
 *             error code can be determined by the implementation.
 *
 * @note Ensure that the `args` array is properly terminated with a NULL pointer.
 *       This function may invoke system calls that can fail, so error handling
 *       should be implemented as needed.
 *
 */
int run_args(char *args[]);


/**
 * @brief Searches for the full path of a command in the system's PATH environment variable.
 *
 * This function takes a command name as input and searches for its executable file
 * in the directories listed in the PATH environment variable. If the command is found,
 * the full path to the executable is returned. If the command is not found, the original
 * command name is returned.
 *
 * @param cmd A pointer to a string representing the command name to search for.
 *            This should be a null-terminated string.
 *
 * @return A pointer to a string containing the full path of the command if found,
 *         or the original command name if not found. The returned string is dynamically
 *         allocated and should be freed by the caller to avoid memory leaks.
 *
 */
char* which(char* cmd);

#endif
