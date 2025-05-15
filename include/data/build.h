#include <stdbool.h>
#ifndef _build_h
#define _build_h
/**
 * @file build.h
 * @brief YMP build context management.
 *
 * This header file defines the structure and functions necessary for managing
 * the context of a YMP (Your Module Package) build. It includes the definition
 * of the `ympbuild` structure, which holds information about the build process,
 * including the context, file path, and header data. This file serves as an
 * interface for initializing, configuring, and executing YMP builds.
 */
/**
 * @struct ympbuild
 * @brief Structure representing a YMP build context.
 *
 * This structure holds the context, path, and header information for a YMP build.
 */
typedef struct {
    char* ctx;    /**< Pointer to the context string. */
    char* path;   /**< Pointer to the path string. */
    char* header; /**< Pointer to the header string. */
} ympbuild;

/**
 * @brief Retrieves a value associated with a given name from the YMP build context.
 *
 * This function searches for a specific name in the YMP build context and returns
 * the corresponding value as a string. If the name is not found, NULL is returned.
 *
 * @param ymp Pointer to a `ympbuild` structure containing the build context.
 * @param name The name of the value to retrieve.
 * 
 * @return A pointer to the value string associated with the given name, or NULL if not found.
 *
 * @note The returned string is managed by the `ympbuild` structure. The caller should not free it.
 */
char* ympbuild_get_value(ympbuild* ymp, const char* name);

/**
 * @brief Retrieves an array of values associated with a given name from the YMP build context.
 *
 * This function searches for a specific name in the YMP build context and returns
 * an array of strings corresponding to the values associated with that name. The
 * last element of the array is a NULL pointer to indicate the end of the array.
 * If the name is not found, NULL is returned.
 *
 * @param ymp Pointer to a `ympbuild` structure containing the build context.
 * @param name The name of the values to retrieve.
 * 
 * @return A pointer to an array of strings (char**) containing the values associated with the given name,
 *         or NULL if not found or if memory allocation fails.
 *
 * @note The caller is responsible for freeing the memory allocated for the array and its contents.
 */
char** ympbuild_get_array(ympbuild* ymp, const char* name);

/**
 * @brief Builds a YMP context from a specified file path.
 *
 * This function initializes the YMP build context by reading data from the specified
 * file path. It returns true if the build was successful, or false if there was an error.
 *
 * @param path The file path from which to build the YMP context.
 * 
 * @return true if the build was successful, false otherwise.
 *
 * @note The function may modify global or static state. Ensure that the path is valid and accessible.
 */
bool build_from_path(const char* path);

char *build_source_from_path(const char* path);
char *build_binary_from_path(const char* path);

/**
 * @brief Executes a specified function within the YMP build context.
 *
 * This function runs a function identified by the given name within the
 * context of the provided YMP build structure. It may perform operations
 * such as building, compiling, or processing based on the function's
 * implementation.
 *
 * @param ymp Pointer to the YMP build context structure.
 * @param name The name of the function to execute.
 * @return int Returns 0 on success, or a negative error code on failure.
 */
int ympbuild_run_function(ympbuild* ymp, const char* name);


#endif
