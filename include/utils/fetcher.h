#ifndef _fetcher_h
#define _fetcher_h
/**
 * @file fetcher.h
 * @brief Header file for the fetch function.
 *
 * This file contains the declaration of the fetch function, which is used
 * to download a file from a specified URL and save it to a local path.
 */

#include <stdbool.h>

/**
 * @brief Downloads a file from a specified URL and saves it to a local path.
 *
 * This function uses libcurl to perform the download. It opens the specified
 * file path for writing and writes the contents of the URL to that file.
 *
 * @param url The URL of the file to download. This should be a valid URL
 *            that points to the resource to be fetched.
 * @param path The local file path where the downloaded file will be saved.
 *             This should be a writable path on the local filesystem.
 *
 * @return true if the file was downloaded successfully, false otherwise.
 *
 * @note The function will return false if the URL is invalid, if the file
 *       cannot be opened for writing, or if the download fails for any reason.
 */
bool fetch(const char* url, const char* path);

#endif

