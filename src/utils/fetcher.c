#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>

#include <core/logger.h>
#include <utils/fetcher.h>
#include <utils/string.h>

#include <config.h>

typedef struct {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    size_t cur_size;
} fetcher;

// Callback function to write data to a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t copy = fwrite(ptr, size, nmemb, ((fetcher*)stream)->fp);
    ((fetcher*)stream)->cur_size += copy;
    return copy;
}

// Function to fetch a file from a URL and save it to a specified path
visible bool fetch(const char* url, const char* path) {
    debug("Fetch: %s -> %s\n", url, path);
    fetcher* fetch = malloc(sizeof(fetcher));

    fetch->curl = curl_easy_init();
    if (fetch->curl) {
        fetch->fp = fopen(path, "wb"); // Open file for writing
        if (fetch->fp == NULL) {
            perror("Failed to open file");
            free(fetch);
            return false; // Return false if file opening fails
        }
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Connection: keep-alive");
        chunk = curl_slist_append(chunk, "DNT: 1");
        chunk = curl_slist_append(chunk, "Sec-GPC: 1");
        chunk = curl_slist_append(chunk, "Ymp: \"NE MUTLU TURKUM DIYENE\"");
        curl_easy_setopt(fetch->curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(fetch->curl, CURLOPT_USERAGENT, build_string("Ymp fetcher/%s", VERSION));
        curl_easy_setopt(fetch->curl, CURLOPT_URL, url); // Set the URL
        curl_easy_setopt(fetch->curl, CURLOPT_WRITEFUNCTION, write_data); // Set the write callback
        curl_easy_setopt(fetch->curl, CURLOPT_WRITEDATA, fetch); // Pass the file pointer to the callback
        curl_easy_setopt(fetch->curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

        // Perform the request
        fetch->res = curl_easy_perform(fetch->curl);
        if (fetch->res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(fetch->res));
            fclose(fetch->fp); // Close the file on error
            free(fetch);
            return false; // Return false if the request fails
        }

        fclose(fetch->fp); // Close the file
        curl_easy_cleanup(fetch->curl); // Clean up
        free(fetch);
        return true; // Return true if the download is successful
    }

    return false; // Return false if curl initialization fails
}

