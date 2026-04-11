#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>

#include <core/ymp.h>
#include <core/logger.h>
#include <utils/fetcher.h>
#include <utils/file.h>
#include <utils/string.h>

#include <config.h>

typedef struct {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    size_t cur_size;
    size_t total_size;
    FetchProgressCB progress_cb;
    void* userdata;
    char* url;
} fetcher;

static size_t write_data(const void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t copy = fwrite(ptr, size, nmemb, ((fetcher*)stream)->fp);
    ((fetcher*)stream)->cur_size += copy;
    return copy;
}

static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    (void)ultotal;
    (void)ulnow;
    fetcher* f = (fetcher*)clientp;
    if (f && f->progress_cb) {
        f->progress_cb(f->url, (size_t)dlnow, (size_t)dltotal, f->userdata);
    }
    return 0;
}

bool fetch_with_progress(const char* url, const char* path, FetchProgressCB cb, void* userdata) {
    debug("Fetch: %s -> %s\n", url, path);
    fetcher* fetch = malloc(sizeof(fetcher));

    if(!fetch) {
        perror("malloc");
        return false;
    }

    fetch->progress_cb = cb;
    fetch->userdata = userdata;
    fetch->url = (char*)url;
    fetch->cur_size = 0;
    fetch->total_size = 0;

    CURL *curl = curl_easy_init();
    fetch->curl = curl;
    if (fetch->curl) {
        char* dir = strdup(path);
        dirname(dir);
        create_dir(dir);
        free(dir);

        fetch->fp = fopen(path, "wb");
        if (fetch->fp == NULL) {
            perror("Failed to open file");
            free(fetch);
            return false;
        }

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Connection: keep-alive");
        chunk = curl_slist_append(chunk, "DNT: 1");
        chunk = curl_slist_append(chunk, "Sec-GPC: 1");
        chunk = curl_slist_append(chunk, "Ymp: \"NE MUTLU TURKUM DIYENE\"");
        curl_easy_setopt(fetch->curl, CURLOPT_HTTPHEADER, chunk);
        char* useragent = build_string("Ymp fetcher/%s", VERSION);
        curl_easy_setopt(fetch->curl, CURLOPT_USERAGENT, useragent);
        curl_easy_setopt(fetch->curl, CURLOPT_URL, url);
        curl_easy_setopt(fetch->curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(fetch->curl, CURLOPT_WRITEDATA, fetch);
        curl_easy_setopt(fetch->curl, CURLOPT_FOLLOWLOCATION, 1L);

        if (cb) {
            curl_easy_setopt(fetch->curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
            curl_easy_setopt(fetch->curl, CURLOPT_XFERINFODATA, fetch);
            curl_easy_setopt(fetch->curl, CURLOPT_NOPROGRESS, 0L);
        }

        fetch->res = curl_easy_perform(fetch->curl);
        if (fetch->res != CURLE_OK) {
            print(_("Download failed: %s\n"), curl_easy_strerror(fetch->res));
            fclose(fetch->fp);
            curl_slist_free_all(chunk);
            free(useragent);
            free(fetch);
            return false;
        }

        curl_off_t clen = 0;
        curl_easy_getinfo(fetch->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &clen);
        fetch->total_size = (size_t)clen;
        if (cb) {
            cb(url, fetch->cur_size, fetch->total_size, userdata);
        }

        fclose(fetch->fp);
        curl_slist_free_all(chunk);
        free(useragent);
        curl_easy_cleanup(fetch->curl);
        free(fetch);
        return true;
    }

    free(fetch);
    return false;
}

visible bool fetch(const char* url, const char* path) {
    return fetch_with_progress(url, path, NULL, NULL);
}

