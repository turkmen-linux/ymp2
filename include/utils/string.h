/**
 * @file string.h
 * @brief Custom string manipulation functions and macros.
 *
 * This header file provides a collection of functions for string manipulation,
 * including string comparison, length calculation, joining strings, and URL encoding/decoding.
 * It also includes macros for character classification.
 */

#ifndef _string_h
#define _string_h

#include <stddef.h>
#include <string.h>

/**
 * @brief Reads the entire content of a file into a string.
 *
 * @param path The path to the file to be read.
 * @return A pointer to the string containing the file content, or NULL if the file could not be opened or read.
 */
char* readfile(const char *path);


/**
 * @brief Compares two strings for equality.
 *
 * @param A The first string to compare.
 * @param B The second string to compare.
 * @return true if the strings are equal, false otherwise.
 */
#define iseq(A, B) (strlen(A) == strlen(B) && (strcmp(A, B) == 0))

/**
 * @brief Counts the number of tab characters in a given data string.
 *
 * @param data The string to search for tab characters.
 * @return The count of tab characters in the string.
 */
long count_tab(char* data);

/**
 * @brief Joins an array of strings into a single string with a specified delimiter.
 *
 * @param f The delimiter to use for joining.
 * @param array The array of strings to join.
 * @return A new string containing the joined strings.
 */
char* join(const char* f, char** array);

/**
 * @brief Concatenates two strings.
 *
 * @param str1 The first string.
 * @param str2 The second string.
 * @return A new string that is the concatenation of str1 and str2.
 */
char* str_add(char* str1, char* str2);

/**
 * @brief Trims whitespace from the beginning and end of a string.
 *
 * @param data The string to trim.
 * @return A new string with whitespace removed from both ends.
 */
char* trim(char* data);

/**
 * @brief Converts an integer to a string.
 *
 * @param num The integer to convert.
 * @return A string representation of the integer.
 */
char* int_to_string(int num);

/**
 * @brief Decodes a URL-encoded string.
 *
 * @param input The URL-encoded string to decode.
 * @return A new string containing the decoded result.
 */
char* url_decode(const char *input);

/**
 * @brief Encodes a string for use in a URL.
 *
 * @param input The string to encode.
 * @return A new string containing the URL-encoded result.
 */
char* url_encode(const char *input);

/**
 * @brief Checks if a string ends with a specified suffix.
 *
 * @param data The string to check.
 * @param f The suffix to check for.
 * @return 1 if data ends with f, 0 otherwise.
 */
#define endswith(data, f) \
    ((strlen(f) <= strlen(data)) && (strncmp(data + strlen(data) - strlen(f), f, strlen(f)) == 0))

/**
 * @brief Checks if a string starts with a specified prefix.
 *
 * @param data The string to check.
 * @param f The prefix to check for.
 * @return 1 if data starts with f, 0 otherwise.
 */
#define startswith(data, f) \
    ((strlen(f) <= strlen(data)) && (strncmp(data, f, strlen(f)) == 0))

/**
 * @brief Builds a formatted string using variable arguments.
 *
 * @param format The format string.
 * @param ... The values to format into the string.
 * @return A new formatted string.
 */
char* build_string(char* format, ...);

// Macros

/** @cond */
#define isalnum_c(A) \
    (A >= 'A' && A <= 'Z') || \
    (A >= 'a' && A <= 'z') || \
    (A >= '0' && A <= '9') || \
    A == '-' || A == '_' || A == '.' || A == '~'

#define isHexDigit(A) \
    (A >= '0' && A <= '9') || \
    (A >= 'A' && A <= 'F') || \
    (A >= 'a' && A <= 'f')

/** @endcond */

/**
 * @brief Replaces all occurrences of a substring within a string with a new substring.
 *
 * This function searches for all occurrences of the specified substring (`oldSub`) 
 * in the input string (`str`) and replaces them with the new substring (`newSub`).
 * The resulting string is dynamically allocated and must be freed by the caller.
 *
 * @param str The original string in which the replacement will occur. This string 
 *            should be null-terminated and not modified by this function.
 * @param oldSub The substring to be replaced. This should also be null-terminated.
 * @param newSub The substring to replace `oldSub` with. This should also be 
 *               null-terminated.
 *
 * @return A pointer to a new string with the replacements made. If no replacements 
 *         are made, a copy of the original string is returned. If memory allocation 
 *         fails, NULL is returned.
 *
 * @note The caller is responsible for freeing the memory allocated for the 
 *       returned string.
 * @warning If `oldSub` is an empty string, the behavior is undefined. Ensure that 
 *          `oldSub` is not empty before calling this function.
 *
 */
char* str_replace(const char* str, const char* oldSub, const char* newSub);

/**
 * @brief Splits a string into an array of strings based on a specified delimiter.
 *
 * This function takes a string and splits it into multiple substrings based on the
 * provided delimiter. The resulting substrings are stored in a dynamically allocated
 * array of strings, which must be freed by the caller to avoid memory leaks.
 *
 * @param data The input string to be split. This string should be null-terminated.
 * @param f The delimiter character used to split the string. 
 *          If this character is not found in the string, the entire string will be returned as the first element.
 * 
 * @return A pointer to an array of strings (char**) containing the split substrings.
 *         The last element of the array is a NULL pointer to indicate the end of the array.
 *         If memory allocation fails, NULL is returned.
 *
 * @note The caller is responsible for freeing the memory allocated for the array and its contents.
 */
char** split(const char* data, const char* f);

/**
 * @brief Removes leading and trailing whitespace from a string.
 *
 * This function takes a string and removes any leading and trailing whitespace characters,
 * including spaces, tabs, and newlines. The modified string is returned.
 *
 * @param str The input string to be stripped of whitespace. This string should be mutable.
 * 
 * @return A pointer to the modified string with leading and trailing whitespace removed.
 *         If the input string is NULL, the function will return NULL.
 *
 * @note The input string is modified in place. Ensure that the string is mutable and not a string literal.
 */
char* strip(const char* str);

#endif // _string_h

