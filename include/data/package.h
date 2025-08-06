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
    bool is_virtual;
    void* repo; /**< Address of repository */
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

/*
 * @brief Loads a package from the provided metadata.
 *
 * This function initializes a Package structure using the metadata string.
 * It can handle both source and binary packages based on the is_source flag.
 *
 * @param pkg Pointer to the Package structure to be initialized.
 * @param metadata A string containing the metadata for the package.
 * @param is_source A boolean flag indicating whether the package is a source package.
 *                  If true, the function treats the metadata as source package data;
 *                  otherwise, it treats it as binary package data.
 */
void package_load_from_metadata(Package* pkg, const char* metadata, bool is_source);

/**
 * @brief Download package from given uri
 *
 * @param pkg A pointer to the Package structure that contains the package
 *            to be downloaded.
 *
 * @return true if the extraction was successful, false otherwise.
 */
bool package_download(Package* pkg, const char* repo_uri);

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


/**
 * @brief Releases the resources associated with a Package.
 *
 * This function decreases the reference count of the specified package.
 * If the reference count reaches zero, the package is freed.
 *
 * @param pkg Pointer to the Package instance to be released.
 */
void package_unref(Package *pkg);

/**
 * @brief Checks if a specified package is installed in the system.
 *
 * This function verifies the installation status of the given package. 
 * It returns true if the package is installed, and false otherwise.
 *
 * @param pkg Pointer to the Package structure that contains the package information.
 *            This structure must be valid and properly initialized before calling this function.
 *
 * @return true if the package is installed; false if it is not installed or if an error occurs.
 *
 * @note Ensure that the package pointer is not NULL before calling this function to avoid undefined behavior.
 */
bool package_is_installed(Package *pkg);


#endif
