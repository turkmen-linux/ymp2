#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <stdbool.h>

#include <utils/fetcher.h>

// Callback function to write data to a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

// Function to fetch a file from a URL and save it to a specified path
visible bool fetch(const char* url, const char* path) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(path, "wb"); // Open file for writing
        if (fp == NULL) {
            perror("Failed to open file");
            return false; // Return false if file opening fails
        }

        curl_easy_setopt(curl, CURLOPT_URL, url); // Set the URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); // Set the write callback
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); // Pass the file pointer to the callback
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fclose(fp); // Close the file on error
            return false; // Return false if the request fails
        }

        fclose(fp); // Close the file
        curl_easy_cleanup(curl); // Clean up
        return true; // Return true if the download is successful
    }

    return false; // Return false if curl initialization fails
}

