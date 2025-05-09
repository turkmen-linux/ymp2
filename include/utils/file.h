#ifndef _file_h
#define _file_h

/**
 * @file file.h
 * @brief Header file for file and directory utilities.
 *
 * This header file contains function declarations for checking file types,
 * creating directories, and obtaining file sizes.
 */

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * @brief Get the size of a file.
 *
 * @param path The path to the file.
 * @return The size of the file in bytes, or 0 if the file does not exist or an error occurs.
 */
uint64_t filesize(const char* path);

/**
 * @brief Check if a given filename is a file.
 *
 * @param filename The name of the file to check.
 * @return true if the file is a file, false otherwise.
 */
#define isfile(filename) (access(filename, F_OK) == 0)

/**
 * @brief Checks if a file or directory exists at the specified path.
 *
 * This function uses the `stat` system call to determine if a file or directory
 * exists at the given path. It returns true if the path exists and false otherwise.
 *
 * @param path representing to check.
 *             The path can be either a file or a directory.
 * @return Returns true (non-zero) if the file or directory exists, 
 *         and false (zero) if it does not exist or an error occurs.
 *
 * @note This function does not differentiate between files and directories;
 *       it simply checks for the existence of the specified path.
 */
bool isexists(const char* path);


/**
 * @brief Check if a given filename is a symbolic link.
 *
 * @param filename The name of the file to check.
 * @return true if the file is a symbolic link, false otherwise.
 */
bool issymlink(const char *filename);

/**
 * @brief Check if a given path is a directory.
 *
 * @param path The path to check.
 * @return true if the path is a directory, false otherwise.
 */
bool isdir(const char *path);

/**
 * @brief Create a directory and any necessary parent directories.
 *
 * @param dir The path of the directory to create.
 */
void create_dir(const char *dir);


char** listdir(const char* path);
char** find(const char* path);

#endif
