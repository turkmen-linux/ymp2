#ifndef _array_h
#define _array_h

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

/**
 * @brief Dynamic array structure.
 *
 * This struct represents a dynamic array that can hold strings.
 * It includes functionality for adding, removing, and manipulating
 * the array elements in a thread-safe manner.
 */
typedef struct {
    char **data;              /**< Pointer to the array of strings. */
    size_t size;              /**< Current number of elements in the array. */
    size_t capacity;          /**< Maximum number of elements the array can hold. */
    size_t removed;           /**< Number of removed elements (for tracking). */
    pthread_mutex_t lock;     /**< Mutex for thread safety. */
} array;

/**
 * @brief Create a new dynamic array.
 *
 * This function allocates and initializes a new dynamic array.
 *
 * @return A pointer to the newly created array, or NULL if the allocation fails.
 */
array* array_new();

/**
 * @brief Add a string to the dynamic array.
 *
 * This function adds a single string to the end of the array.
 *
 * @param arr Pointer to the dynamic array.
 * @param data The string to add to the array.
 */
void array_add(array *arr, char* data);

/**
 * @brief Add multiple strings to the dynamic array.
 *
 * This function adds an array of strings to the end of the dynamic array.
 *
 * @param arr Pointer to the dynamic array.
 * @param data Pointer to the array of strings to add.
 * @param len The number of strings to add.
 */
void array_adds(array *arr, char** data, size_t len);

/**
 * @brief Set the contents of the dynamic array.
 *
 * This function replaces the current contents of the array with new data.
 *
 * @param arr Pointer to the dynamic array.
 * @param new_data Pointer to the new array of strings.
 * @param len The number of strings in the new data.
 */
void array_set(array *arr, char** new_data, size_t len);

/**
 * @brief Get the contents of the dynamic array.
 *
 * This function retrieves the current contents of the array.
 *
 * @param arr Pointer to the dynamic array.
 * @param len Pointer to a variable to store the length of the array.
 * @return Pointer to the array of strings.
 */
char **array_get(array *arr, int* len);

/**
 * @brief Get a single concatenated string from the dynamic array.
 *
 * This function concatenates all strings in the array into a single string.
 *
 * @param arr Pointer to the dynamic array.
 * @return A single concatenated string, or NULL if the array is empty.
 */
char *array_get_string(array *arr);

/**
 * @brief Get the number of elements in the dynamic array.
 *
 * @param arr Pointer to the dynamic array.
 * @return The number of elements in the array.
 */
size_t array_length(array *arr);

/**
 * @brief Reverse the order of elements in the dynamic array.
 *
 * This function reverses the order of the strings in the array.
 *
 * @param arr Pointer to the dynamic array.
 */
void array_reverse(array *arr);

/**
 * @brief Remove duplicate elements from the dynamic array.
 *
 * This function removes duplicate strings from the array, keeping only unique values.
 *
 * @param arr Pointer to the dynamic array.
 */
void array_uniq(array *arr);

/**
 * @brief Insert a string at a specific index in the dynamic array.
 *
 * This function inserts a string at the specified index, shifting subsequent elements.
 *
 * @param arr Pointer to the dynamic array.
 * @param value The string to insert.
 * @param index The index at which to insert the string.
 */
void array_insert(array *arr, char* value, size_t index);

/**
 * @brief Sort the elements of the dynamic array.
 *
 * This function sorts the strings in the array in ascending order.
 *
 * @param arr Pointer to the dynamic array.
 */
void array_sort(array *arr);

/**
 * @brief Release the resources used by the dynamic array.
 *
 * This function frees the memory allocated for the array and its elements.
 *
 * @param arr Pointer to the dynamic array to free.
 */
void array_unref(array *arr);

/**
 * @brief Remove an element at a specific index from the dynamic array.
 *
 * This function removes the string at the specified index, shifting subsequent elements.
 *
 * @param arr Pointer to the dynamic array.
 * @param index The index of the element to remove.
 */
void array_pop(array *arr, size_t index);

/**
 * @brief Check if an item exists in the dynamic array.
 *
 * This function checks if a specific string is present in the array.
 *
 * @param arr Pointer to the dynamic array.
 * @param item The string to check for existence in the array.
 * @return `true` if the item exists in the array, `false` otherwise.
 */
bool array_has(array *arr, char* item);

/**
 * @brief Remove a specific item from the dynamic array.
 *
 * This function removes the first occurrence of the specified string from the array.
 *
 * @param arr Pointer to the dynamic array.
 * @param item The string to remove from the array.
 */
void array_remove(array* arr, char* item);

#endif
