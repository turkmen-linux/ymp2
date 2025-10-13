#include <data/repository.h>
#include <data/package.h>

/**
 * @brief Resolves the dependencies for a given package name.
 *
 * This function takes the name of a package and returns an array of strings
 * representing the names of the dependencies required by that package.
 * The caller is responsible for freeing the returned array and its contents.
 *
 * @param name The name of the package for which to resolve dependencies.
 * @return A pointer to an array of strings containing the dependency names,
 *         or NULL if the package is not found or an error occurs.
 *
 * @code
 * char* package_name = "example-package";
 * Package** dependencies = resolve_dependency(package_name);
 * if (dependencies) {
 *     for (int i = 0; dependencies[i] != NULL; i++) {
 *         printf("Dependency: %s\n", dependencies[i]);
 *     }
 * } else {
 *     printf("No dependencies found for package: %s\n", package_name);
 * }
 * @endcode
 */
Package** resolve_dependency(char* name);

char** get_group_packages (const char* name);

Package** resolve_reverse_dependency(char* name);

/**
 * @brief Loads repositories for dependency resolution.
 *
 * This function initiates the process of loading repositories that are required
 * for resolving dependencies. It prepares the necessary resources and returns
 * a pointer to the loaded repositories. The caller is responsible for managing
 * the lifecycle of the returned repository pointer.
 *
 * @return A pointer to the loaded Repository object. 
 *         Returns NULL if the loading process could not be initiated.
 *
 * @code
 * Repository** repos = resolve_begin();
 * if (repos) {
 *     // Proceed with dependency resolution
 *     // ...
 *     resolve_end(repos); // Free the resources after use
 * } else {
 *     printf("Failed to load repositories.\n");
 * }
 * @endcode
 */
Repository** resolve_begin();

/**
 * @brief Frees the loaded repositories after dependency resolution.
 *
 * This function finalizes the dependency resolution process and frees any
 * resources associated with the loaded repositories. The caller must ensure
 * that the repository pointer passed to this function is valid and was obtained
 * from a previous call to `resolve_begin()`.
 *
 * @param repos A pointer to a pointer of the Repository object to be freed.
 *              This should be the same pointer returned by `resolve_begin()`.
 *
 * @code
 * // Assuming repos was obtained from resolve_begin()
 * resolve_end(repos);
 * @endcode
 */
void resolve_end(Repository** repos);

