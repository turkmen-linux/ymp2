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

typedef void (*FetchProgressCallback)(const char* url, size_t downloaded, size_t total, void* userdata);

bool fetch_with_progress(const char* url, const char* path, FetchProgressCallback cb, void* userdata);
#define fetch(A, B) fetch_with_progress(A, B, NULL, NULL)

#endif

