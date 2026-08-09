// Override libc functions
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef MUSL
typedef ssize_t (*write_func_t)(int, const void *, size_t);
static write_func_t original_write;

visible ssize_t write(int fd, const void *buf, size_t count) {
    if (!original_write) {
        original_write = dlsym(RTLD_NEXT, "write");
    }
/* Splitting the buffer into 100MB chunks */
#define max_chunk_size (100 * 1024 * 1024) /* 100MB */
    size_t bytes_written = 0;
    while (count > 0) {
        size_t chunk_size = (count > max_chunk_size) ? max_chunk_size : count;
        ssize_t result = original_write(fd, buf + bytes_written, chunk_size);
        if (result < 0) {
            /* Error occurred, return immediately */
            return result;
        }
        bytes_written += result;
        count -= result;
        if (chunk_size == max_chunk_size) {
            fsync(fd);
        }
    }
    return bytes_written;
}

typedef void *(*malloc_func_t)(int);
static malloc_func_t original_malloc;

/* malloc buffer default filled by zero */
visible void *malloc(size_t size) {
    if (!original_malloc) {
        original_malloc = dlsym(RTLD_NEXT, "malloc");
    }
    void *buf = original_malloc(size);
    memset(buf, 0, size);
    return buf;
}

#endif