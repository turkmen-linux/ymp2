#ifndef _yaml_h
#define _yaml_h

#include <stdbool.h>

/**
 * @file yaml.h
 * @brief yaml parser
 */

/**
 * @brief Checks if a specific area exists in the data.
 *
 * @param data The data to search within.
 * @param path The name of the area to check for.
 * @return true if the area exists, false otherwise.
 */
bool yaml_has_area(const char *data, const char *path);

/**
 * @brief Gets the content of a specific area from the data.
 *
 * @param data The data to search within.
 * @param path The name of the area to retrieve.
 * @return A pointer to the string containing the area content, or NULL if the area does not exist.
 */
char* yaml_get_area(const char *data, const char *path);

/**
 * @brief Gets the value associated with a specific name in the data.
 *
 * @param data The data to search within.
 * @param name The name of the value to retrieve.
 * @return A pointer to the string containing the value, or NULL if the value does not exist.
 */
char* yaml_get_value(const char *data, const char *name);

/**
 * @brief Gets an array of values associated with a specific name in the data.
 *
 * @param data The data to search within.
 * @param name The name of the values to retrieve.
 * @param count A pointer to an integer that will be set to the number of values retrieved.
 * @return A pointer to an array of strings containing the values, or NULL if the name does not exist.
 */
char** yaml_get_array(const char *data, const char *name, int *count);

/**
 * @brief Gets a list of areas from the data.
 *
 * @param fdata The data to search within.
 * @param path The name of the area to retrieve.
 * @param area_count A pointer to an integer that will be set to the number of areas retrieved.
 * @return A pointer to an array of strings containing the areas, or NULL if the area does not exist.
 */
char** yaml_get_area_list(const char* fdata, const char* path, int* area_count);

#endif
