/**
 * @file quarantine.h
 * @brief Header file for managing package quarantine in the package manager.
 *
 * This header file defines functions for validating, resetting, and installing
 * packages in a quarantine state. It is essential for maintaining system
 * integrity and security by ensuring that potentially harmful packages are
 * properly managed.
 *
 */

#ifndef QUARANTINE_H
#define QUARANTINE_H

/**
 * @brief Validates the quarantine status of packages in the package manager.
 *
 * This function checks whether the packages are in a valid quarantine state.
 * It ensures that the packages meet the necessary criteria for quarantine,
 * such as being properly isolated and not affecting the overall system stability.
 *
 * @return bool Returns true if the packages are valid and in quarantine,
 *              false otherwise.
 *
 * @note This function is crucial for maintaining system integrity and security
 *       by ensuring that potentially harmful packages are properly managed.
 *
 * @warning Ensure that the package manager is initialized before calling this
 *          function to avoid undefined behavior.
 *
 */
bool quarantine_validate();

/**
 * @brief Installs verified packages into the system.
 *
 * This function takes the name of a verified package and installs it into
 * the system, ensuring that only packages that have passed validation are
 * allowed for installation. This helps maintain system integrity and security
 * by preventing the installation of potentially harmful software.
 *
 * @param name The name of the package to be installed.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 *
 * @note This function is crucial for managing package installations and
 *       ensuring that only trusted packages are added to the system.
 *
 * @warning Ensure that the package manager is initialized and that the
 *          package is verified before calling this function to avoid
 *          undefined behavior.
 *
 */
int quarantine_sync(const char* name);


/**
 * @brief Resets the quarantine status of packages in the package manager.
 *
 * This function removes the existing quarantine directory and recreates it,
 * effectively clearing the quarantine status of all packages. This allows
 * previously quarantined packages to be re-evaluated for installation or use,
 * restoring their original state.
 *
 * @return void This function does not return a value.
 *
 * @note This function is essential for managing package states, especially
 *       when a package has been incorrectly quarantined or after a security
 *       incident has been resolved.
 *
 * @warning Use this function with caution, as it may expose the system to
 *          previously quarantined packages that could be harmful.
 *
 */
void quarantine_reset();
#endif