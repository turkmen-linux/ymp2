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
 * @brief Retrieves the directory name from a given path.
 *
 * This function takes a file path as input and returns the directory portion of the path.
 * If the path is NULL or empty, it returns the current directory ("."). If the path does not
 * contain any directory separators ('/'), it also returns the current directory. If the last
 * character of the path is a directory separator, it returns the parent directory.
 *
 * @param path A pointer to a null-terminated string representing the file path.
 *             This string is modified in place to null-terminate at the last '/' character.
 *
 * @return A pointer to the directory name. The return value is:
 *         - "." if the path is NULL, empty, or does not contain a '/'.
 *         - "/" if the path starts with '/'.
 *         - The modified input path up to the last '/' otherwise.
 *
 * @note The input string is modified by null-terminating it at the last '/' character.
 *       Ensure that the caller does not use the original path after this function call.
 */
char* dirname(const char* path);

/**
 * @brief Create a directory and any necessary parent directories.
 *
 * @param dir The path of the directory to create.
 */
void create_dir(const char *dir);

#endif
