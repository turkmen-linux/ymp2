#ifndef _hash_h
#define _hash_h


#define SHA512 0
#define SHA256 1
#define SHA1   2
#define MD5    3

/**
 * @brief Calculates a hash of a file using the specified algorithm.
 *
 * Supported hash types: SHA512 (0), SHA256 (1), SHA1 (2), MD5 (3).
 *
 * @param type The hash algorithm type constant.
 * @param path The path to the file to hash.
 * @return A dynamically allocated hexadecimal hash string,
 *         or NULL on failure. The caller must free the returned string.
 *
 * @code
 * char *hash = calculate_hash(SHA256, "/etc/hostname");
 * printf("SHA256: %s\n", hash);
 * free(hash);
 * @endcode
 */
char *calculate_hash(int type, const char *path);

/**
 * @file hash.h
 * @brief Calculates the SHA-1 hash of a file.
 *
 * This function reads the contents of the file located at the specified path
 * and computes its SHA-1 hash.
 *
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the file for which the SHA-1 hash will be calculated.
 *
 * @return A pointer to a string containing the SHA-1 hash in hexadecimal format,
 *         or NULL if the calculation fails (e.g., due to file access issues or
 *         memory allocation failure). The caller is responsible for freeing the
 *         returned string.
 */
#define calculate_sha1(path) calculate_hash(SHA1, path);

/**
 * @brief Calculates the MD5 hash of a file.
 *
 * This function reads the contents of the file located at the specified path
 * and computes its MD5 hash.
 *
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the file for which the MD5 hash will be calculated.
 *
 * @return A pointer to a string containing the MD5 hash in hexadecimal format,
 *         or NULL if the calculation fails (e.g., due to file access issues or
 *         memory allocation failure). The caller is responsible for freeing the
 *         returned string.
 */
#define calculate_md5(path) calculate_hash(MD5, path);

/**
 * @brief Calculates the SHA-256 hash of a file.
 *
 * This function reads the contents of the file located at the specified path
 * and computes its SHA-256 hash.
 *
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the file for which the SHA-256 hash will be calculated.
 *
 * @return A pointer to a string containing the SHA-256 hash in hexadecimal format,
 *         or NULL if the calculation fails (e.g., due to file access issues or
 *         memory allocation failure). The caller is responsible for freeing the
 *         returned string.
 */
#define calculate_sha256(path) calculate_hash(SHA256, path);

/**
 * @brief Calculates the SHA-512 hash of a file.
 *
 * This function reads the contents of the file located at the specified path
 * and computes its SHA-512 hash.
 *
 * @param path A pointer to a null-terminated string that specifies the path
 *             to the file for which the SHA-512 hash will be calculated.
 *
 * @return A pointer to a string containing the SHA-512 hash in hexadecimal format,
 *         or NULL if the calculation fails (e.g., due to file access issues or
 *         memory allocation failure). The caller is responsible for freeing the
 *         returned string.
 */
#define calculate_sha512(path) calculate_hash(SHA512, path);

#endif
