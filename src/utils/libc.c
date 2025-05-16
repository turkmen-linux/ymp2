// Override libc functions
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>


typedef ssize_t (*write_func_t)(int, const void *, size_t);
static write_func_t original_write;

visible ssize_t write(int fd, const void *buf, size_t count) {
    if (!original_write) {
        original_write = dlsym(RTLD_NEXT, "write");
    }
    /* Splitting the buffer into 5MB chunks */
    #define max_chunk_size (100 * 1024 * 1024) /* 5MB */
    size_t bytes_written = 0;
    ssize_t result;
    while (count > 0) {
        size_t chunk_size = (count > max_chunk_size) ? max_chunk_size : count;
        result = original_write(fd, buf + bytes_written, chunk_size);
        if (result < 0) {
            /* Error occurred, return immediately */
            return result;
        }
        bytes_written += result;
        count -= result;
        if(chunk_size == max_chunk_size) {
            fsync(fd);
        }
    }
    return bytes_written;
}


visible void* malloc(size_t size){
    return (void*) calloc((size_t)1, size);
}
