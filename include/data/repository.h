#ifndef _repository_h
#define _repository_h

#include <data/package.h>

/**
 * @struct Repository
 * @brief Represents a package repository.
 *
 * This structure holds the URI of the repository and an array of packages
 * contained within it, along with the count of those packages.
 */
typedef struct {
    const char* uri;         /**< The URI of the repository. */
    const char* name;         /**< The name of the repository. */
    Package** packages;      /**< Array of pointers to packages in the repository. */
    size_t package_count;    /**< The number of packages in the repository. */
} Repository;

/**
 * @brief Creates a new Repository instance.
 *
 * @return A pointer to the newly created Repository, or NULL on failure.
 */
Repository* repository_new();

/**
 * @brief Loads packages from an index file into the repository.
 *
 * @param repo Pointer to the Repository instance.
 * @param path The path to the index file.
 */
void repository_load_from_index(Repository* repo, const char* path);

/**
 * @brief Loads packages from a data string into the repository.
 *
 * @param repo Pointer to the Repository instance.
 * @param data The data string containing package information.
 */
void repository_load_from_data(Repository* repo, const char* data);

/**
 * @brief Downloads a package from the repository.
 *
 * @param repo Pointer to the Repository instance.
 * @param name The name of the package to download.
 * @param is_source Indicates whether the package is a source package.
 * @return true if the download was successful, false otherwise.
 */
bool repository_download_package(Repository* repo, const char* name, bool is_source);

/**
 * @brief Retrieves a package from the repository.
 *
 * @param repo Pointer to the Repository instance.
 * @param name The name of the package to retrieve.
 * @param is_source Indicates whether to retrieve a source package.
 * @return A pointer to the Package if found, or NULL if not found.
 */
Package* repository_get(Repository *repo, const char* name, bool is_source);

/**
 * @brief Releases the resources associated with the Repository.
 *
 * @param repo Pointer to the Repository instance to be released.
 */
void repository_unref(Repository* repo);

#endif

