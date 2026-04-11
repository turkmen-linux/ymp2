/**
 * @file gui.h
 * @brief Terminal-based GUI utilities for user interaction and progress tracking.
 *
 * Provides functions for displaying messages, interactive yes/no prompts,
 * and multi-progress bar management in a terminal environment.
 */

#ifndef GUI_H
#define GUI_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Message type for gui_msg() display.
 *
 * Determines the visual styling and icon used when displaying a message.
 */
typedef enum {
    MSG_INFO,    /**< Informational message (neutral styling) */
    MSG_SUCCESS, /**< Success message (positive/confirmation styling) */
    MSG_WARNING, /**< Warning message (caution styling) */
    MSG_ERROR    /**< Error message (negative/alert styling) */
} msg_type_t;

/**
 * @brief Initialize the GUI subsystem.
 * @return true on success, false on failure.
 *
 * Must be called before any other GUI functions.
 * Sets up terminal capabilities and internal state.
 */
bool gui_init(void);

/**
 * @brief Clean up the GUI subsystem.
 *
 * Restores terminal to original state and releases resources.
 * Should be called before program exit.
 */
void gui_end(void);

/**
 * @brief Display a styled message to the user.
 * @param title Short title/label for the message.
 * @param msg Full message text to display.
 * @param type Message type determining visual style.
 *
 * Displays the message with appropriate color coding based on type.
 */
void gui_msg(const char *title, const char *msg, msg_type_t type);

/**
 * @brief Prompt the user with a yes/no question.
 * @param title Title displayed with the prompt.
 * @param msg Question text to display.
 * @param def Default selection if user presses Enter without typing.
 * @return true if user selected yes, false if no.
 *
 * Blocks until user provides input. Supports keyboard shortcuts (y/n).
 */
bool gui_yes_no(const char *title, const char *msg, bool def);

/**
 * @brief Maximum number of concurrent progress bars supported.
 */
#define GUI_MAX_BARS 10

/**
 * @brief Add a new progress bar.
 * @param id Unique identifier for the progress bar (used in subsequent calls).
 * @param title Short title for the progress bar.
 * @param msg Current status message.
 * @param total Total number of units to complete.
 * @return Non-negative bar ID on success, -1 on failure.
 *
 * Creates a new progress bar entry. Call gui_progress_update() to modify
 * and gui_progress_remove() when complete.
 */
int gui_progress_add(const char *id, const char *title, const char *msg, size_t total);

/**
 * @brief Update progress bar state.
 * @param id Identifier matching a previously added progress bar.
 * @param done Number of units completed.
 * @param total Current total (may differ from initial total).
 *
 * Updates the displayed progress. Call gui_progress_draw() after updates
 * to refresh the display.
 */
void gui_progress_update(const char *id, size_t done, size_t total);

/**
 * @brief Remove a progress bar.
 * @param id Identifier of the progress bar to remove.
 *
 * Cleans up and removes the specified progress bar from tracking.
 */
void gui_progress_remove(const char *id);

/**
 * @brief Redraw all active progress bars.
 *
 * Refreshes the terminal display showing all active progress bars.
 * Should be called after any gui_progress_add() or gui_progress_update()
 * to ensure changes are visible.
 */
void gui_progress_draw(void);


#endif
