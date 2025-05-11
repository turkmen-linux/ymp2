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


/**
 * @brief Lists the contents of a directory.
 *
 * This function retrieves the names of all files and subdirectories in the
 * specified directory path.
 *
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the directory to be listed.
 *
 * @return A null-terminated array of strings, where each string is the name
 *         of a file or subdirectory in the specified directory. The caller
 *         is responsible for freeing the memory allocated for the array and
 *         its contents. Returns NULL if the directory cannot be accessed or
 *         if an error occurs.
 */
char** listdir(const char* path);

/**
 * @brief Finds files in a specified directory.
 *
 * This function searches for files in the specified directory path and
 * returns their names. The search criteria can be defined within the
 * implementation of this function.
 *
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the directory in which to search for files.
 *
 * @return A null-terminated array of strings, where each string is the name
 *         of a file found in the specified directory. The caller is responsible
 *         for freeing the memory allocated for the array and its contents.
 *         Returns NULL if the directory cannot be accessed or if an error occurs.
 */
char** find(const char* path);

/**
 * @brief Executes a command with arguments and captures its output.
 *
 * This function creates a child process to execute the specified command
 * using the execvp() function. The output of the command is captured and
 * returned as a dynamically allocated string. The caller is responsible for
 * freeing the returned string after use.
 *
 * @param argv An array of command-line arguments, where the first element
 *             is the command to execute, followed by its arguments, and
 *             the last element must be NULL to indicate the end of the array.
 *             Example: {"ls", "-l", NULL}.
 *
 * @return A pointer to a dynamically allocated string containing the output
 *         of the command. If the command fails to execute or if there is
 *         an error in creating the pipe or forking the process, the function
 *         returns NULL. In case of memory allocation failure, the program
 *         will terminate with an error message.
 *
 * @note The output string is null-terminated. The caller must free the
 *       returned string using free() to avoid memory leaks.
 *
 * @warning This function does not handle commands that require user input
 *          or commands that may block indefinitely.
 */
char* getoutput(char* argv[]);


#endif
