#ifndef _archive_h
#define _archive_h

#include <stdbool.h>

#include <utils/array.h>

/** @file archive.h
 * @brief extract or create archives
 */

/**
 * @struct Archive
 * @brief Represents an archive with associated metadata and configuration.
 *
 * This structure holds information related to an archive, including
 * the archive object, paths, and configuration settings for adding
 * files to the archive.
 */
typedef struct {
    struct archive *archive;      /**< Pointer to the archive object. */
    array* errors;               /**< Archive error message array */
    char *archive_path;          /**< Path to the archive file. */
    char *target_path;           /**< Path to the target directory or file. */
    int add_list_size;           /**< Size of the list of files to be added to the archive. */
    int aformat;                 /**< Format of the archive (e.g., zip, tar). */
    int afilter;                 /**< Filter options for the archive (e.g., compression level). */
    /** @cond */
    array *a;                    /** Pointer to an array structure containing files to be added. */
    /** @endcond */
} Archive;


#define zip 0
#define tar 1
#define p7zip 2
#define cpio 3
#define ar 4

#define filter_none 0
#define filter_gzip 1
#define filter_xz 2

/**
 * @brief Creates a new Archive instance.
 *
 * @return A pointer to the newly created Archive.
 */
Archive* archive_new();

/**
 * @brief Loads an archive from the specified path.
 *
 * @param data Pointer to the Archive instance.
 * @param path Path to the archive file.
 */
void archive_load(Archive *data, const char* path);

/**
 * @brief Sets the target extraction path for the archive.
 *
 * @param data Pointer to the Archive instance.
 * @param target Path where the archive will be extracted.
 */
void archive_set_target(Archive *data, const char* target);

/**
 * @brief Checks if the specified path is a valid archive.
 *
 * @param data Pointer to the Archive instance.
 * @param path Path to check.
 * @return true if the path is a valid archive, false otherwise.
 */
bool archive_is_archive(Archive *data, const char *path);

/**
 * @brief Lists the files contained in the archive.
 *
 * @param data Pointer to the Archive instance.
 * @param len Pointer to an integer to store the number of files.
 * @return An array of file names contained in the archive.
 */
char** archive_list_files(Archive *data, size_t* len);

/**
 * @brief Adds a file path to the archive for future writing.
 *
 * @param data Pointer to the Archive instance.
 * @param path Path of the file to add.
 */
void archive_add(Archive *data, const char *path);

/**
 * @brief Creates an archive with the specified files.
 *
 * @param data Pointer to the Archive instance.
 */
void archive_create(Archive *data);

/**
 * @brief Reads a file from the archive.
 *
 * @param data Pointer to the Archive instance.
 * @param file_path Path of the file to read from the archive.
 * @return A string containing the file's contents, or NULL on failure.
 */
char* archive_readfile(Archive *data, const char *file_path);

/**
 * @brief Sets the type and filter for the archive.
 *
 * @param data Pointer to the Archive instance.
 * @param form The format of the archive (e.g., "zip", "tar").
 * @param filt The filter to apply (e.g., "none", "gzip").
 */
void archive_set_type(Archive *data, const char* form, const char* filt);

/**
 * @brief Writes files to the archive.
 *
 * @param data Pointer to the Archive instance.
 * @param outname Name of the output archive file.
 * @param filename Array of file names to include in the archive.
 */
void archive_write(Archive *data, const char *outname, char **filename);

/**
 * @brief Extracts all files from the archive.
 *
 * @param data Pointer to the Archive instance.
 */
void archive_extract_all(Archive *data);

/**
 * @brief Extracts a specific file from the archive.
 *
 * @param data Pointer to the Archive instance.
 * @param path Path of the file to extract.
 */
void archive_extract(Archive *data, const char* path);

#endif
