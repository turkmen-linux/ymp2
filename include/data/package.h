#ifndef _package_h
#define _package_h
#include <utils/archive.h>

/**
 * @file package.h
 * @brief Package data management
 *
 * This library provides functions to store and load package information.
 */

/**
 * @struct Package
 * @brief Structure to hold package information
 *
 * This structure stores various details about a package, including its name,
 * version, dependencies, and other metadata.
 */
typedef struct {
    const char* name; /**< Package name */
    char** dependencies; /**< List of package dependencies */
    const char* version; /**< Package version string */
    int release; /**< Package release number */
    bool is_source; /**< Indicates if the package is a source package */
    
    /** @cond */
    const char* metadata; /**< Package metadata. Used by internal functions. Do not modify! */
    const char* files; /**< Package files metadata. Used by internal functions. Do not modify! */
    const char* links; /**< Package links metadata. Used by internal functions. Do not modify! */
    array *errors; /**< List of errors encountered during package processing */
    Archive *archive; /**< Pointer to the package archive */
    /** @endcond */
} Package;

/**
 * @brief Initializes a new Package structure.
 *
 * This function allocates memory for a new Package structure and initializes
 * its members to default values.
 *
 * @return A pointer to the newly created Package structure, or NULL if
 *         the initialization fails (e.g., due to memory allocation failure).
 */
Package* package_new();

/**
 * @brief Loads a package from a specified file.
 *
 * This function reads the contents of the file located at the given path
 * and initializes the provided Package structure with the data from the file.
 * It is expected that the file is in a specific format that the function can parse.
 *
 * @param pkg A pointer to a Package structure that will be populated with
 *            the data loaded from the file. This structure must be allocated
 *            before calling this function.
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the file from which the package will be loaded. The function
 *             will attempt to open this file for reading.
 *
 * @note Ensure that the Package structure is properly initialized before
 *       calling this function. The caller is responsible for managing the
 *       memory of the Package structure.
 *
 * @warning This function may modify the contents of the Package structure
 *          if loading is successful. Ensure that the structure is in a valid
 *          state before calling this function.
 */
void package_load_from_file(Package* pkg, const char* path);

void package_load_from_metadata(Package* pkg, const char* metadata, bool is_source);

/**
 * @brief Extracts the contents of a package.
 *
 * This function extracts the contents of the specified package. The exact
 * behavior of the extraction process depends on the implementation.
 *
 * @param pkg A pointer to the Package structure that contains the package
 *            to be extracted.
 *
 * @return true if the extraction was successful, false otherwise.
 */
bool package_extract(Package* pkg);


void package_unref(Package *pkg);
#endif
