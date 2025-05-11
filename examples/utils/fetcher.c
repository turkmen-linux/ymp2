#include <stdio.h>

#include <utils/fetcher.h>

// Main function to demonstrate usage of fetch
int main(void) {
    const char *url = "http://example.com"; // URL of the file to download
    const char *path = "index.html"; // Output filename

    // Call the fetch function
    if (fetch(url, path)) {
        printf("File downloaded successfully to %s\n", path);
    } else {
        printf("Failed to download file from %s\n", url);
    }

    return 0;
}
