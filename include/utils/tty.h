/**
 * @file tty.h
 * @brief Raw terminal mode management.
 */

/**
 * @brief Disables raw terminal mode and restores original settings.
 */
void disable_raw_mode(void);

/**
 * @brief Enables raw terminal mode for direct character input.
 */
void enable_raw_mode(void);

/**
 * @brief Sets up raw terminal mode.
 *
 * @return 0 on success, or -1 if an error occurs.
 */
int setup_raw_mode();