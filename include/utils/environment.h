/**
 * @file environment.h
 * @brief Environment variable save, clear, and restore.
 */

#ifndef _environment_h
#define _environment_h

/**
 * @brief Saves the current environment settings.
 *
 * This function captures the current state of the environment,
 * including any relevant configurations or data that might be needed
 * for restoring the environment later. Typically used before making
 * changes that could alter the environment's state.
 *
 * @note This function may be used in contexts where the environment
 *       needs to be reverted back to a prior state.
 */
void save_env();

/**
 * @brief Clears the current environment settings.
 *
 * This function removes or resets the current environment settings
 * to their default or initial state. It is useful for ensuring that
 * subsequent operations are performed in a clean environment.
 *
 * @warning Use with caution, as clearing the environment may result
 *          in loss of any unsaved changes or settings.
 */
void clear_env();

/**
 * @brief Restores the previously saved environment settings.
 *
 * This function reverts the environment to a state that was captured
 * previously by the `save_env()` function. It restores all configurations
 * and data that were active at the time of saving.
 *
 * @note It is important to call `save_env()` before this function
 *       to ensure that there is a valid state to restore from.
 */
void restore_env();

#endif
