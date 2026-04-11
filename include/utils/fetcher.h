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

typedef struct FetchJob FetchJob;

typedef void (*FetchProgressCB)(const char* url, size_t downloaded, size_t total, void* userdata);

struct FetchJob {
    char* url;
    char* path;
    size_t downloaded;
    size_t total;
    bool done;
    bool error;
    char* error_msg;
};

bool fetch_with_progress(const char* url, const char* path, FetchProgressCB cb, void* userdata);
bool fetch(const char* url, const char* path);

#endif

