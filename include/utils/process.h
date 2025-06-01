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
 * This function retrieves the current epoch time in seconds.
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

#endif
