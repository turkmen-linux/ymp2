#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <utils/array.h>
#include <core/logger.h>

#define csort(A, B) qsort(A, B, sizeof(const char*), (int (*)(const void *, const void *))strcmp)

visible array *array_new() {
    array *arr = (array *)malloc(sizeof(array));
    if(!arr){
        printf("memory allocation failed");
        return NULL;
    }
    arr->data = (char**)malloc(1024* sizeof(char*));
    if(!arr->data){
        printf("memory allocation failed");
        free(arr);
        return NULL;
    }
    arr->size = 0;
    arr->capacity = 1024;
    arr->removed = 0;
    arr->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    size_t start;
    for(start=0;start<arr->capacity;start++){
        arr->data[start] = NULL;
    }
    return arr;
}

visible void array_add(array *arr, const char *value) {
    pthread_mutex_lock(&arr->lock);
    
    // Check if we need to increase capacity
    if (arr->size >= arr->capacity) {
        arr->capacity += 1024;
        arr->data = (char **)realloc(arr->data, arr->capacity * sizeof(char *));
        if (!arr->data) {
            // Handle memory allocation failure
            pthread_mutex_unlock(&arr->lock);
            return;
        }
        // Initialize new slots to NULL
        for (size_t start = arr->size; start < arr->capacity; start++) {
            arr->data[start] = NULL;
        }
    }

    // Free the previous value if it exists
    if (arr->data[arr->size]) {
        free(arr->data[arr->size]);
    }

    // Add the new value
    arr->data[arr->size] = strdup(value);
    arr->size++;

    pthread_mutex_unlock(&arr->lock);
}

visible void array_set(array *arr, char** new_data){
    pthread_mutex_lock(&arr->lock);
    arr->data = calloc(arr->capacity, sizeof(char*));
    arr->size = 0;
    arr->removed = 0;
    size_t start;
    for(start=0;start<arr->capacity;start++){
            arr->data[start] = NULL;
    }
    pthread_mutex_unlock(&arr->lock);
    array_adds(arr, new_data);
}

visible char* array_get_string(array *arr){
    pthread_mutex_lock(&arr->lock);
    size_t tot_len = 0;
    size_t start = 0;
    while(start < arr->capacity){
        if(arr->data[start] != NULL){
            tot_len += strlen(arr->data[start]);
        }
        start++;
    }
    char* ret = calloc(tot_len+1, sizeof(char));
    start = 0;
    while(start < arr->size+arr->removed+1){
        if(arr->data[start] != NULL){
            strcat(ret, arr->data[start]);
        }
        start++;
    }
    pthread_mutex_unlock(&arr->lock);
    return ret;
}

visible void array_adds(array *arr, char **value) {
    for(size_t i=0;value[i];i++){
        array_add(arr, value[i]);
    }
}

visible void array_remove(array* arr, const char* item){
    pthread_mutex_lock(&arr->lock);
    size_t start = 0;
    while(start < arr->capacity){
        if(arr->data[start] != NULL && strcmp(arr->data[start],item)==0){
            arr->data[start] = NULL;
            arr->size -= 1;
            arr->removed +=1;
        }
        start++;
    }
    pthread_mutex_unlock(&arr->lock);
}

visible bool array_has(array* arr, const char* name){
    pthread_mutex_lock(&arr->lock);
    size_t start = 0;
    while(start < arr->size + arr->removed){
        if(arr->data[start]){
           if (strcmp(arr->data[start], name) == 0){
               pthread_mutex_unlock(&arr->lock);
               return true;
           }
        }
        start++;
    }
    pthread_mutex_unlock(&arr->lock);
    return false;
}

visible void array_uniq(array* arr){
    pthread_mutex_lock(&arr->lock);
    size_t start = 1;
    size_t i = 0;
    size_t removed = 0;
    while(start < arr->capacity){
        if(arr->data[start] == NULL){
            start++;
            continue;
        }
        for(i=0;i<start-1;i++){
             if(arr->data[i] != NULL && strcmp(arr->data[i],arr->data[start])==0){
                 arr->data[i] = NULL;
                 removed++;
            }
        }
        start++;
    }
    arr->size -= removed;
    arr->removed = removed;
    pthread_mutex_unlock(&arr->lock);
}

visible void array_pop(array* arr, size_t index){
    pthread_mutex_lock(&arr->lock);
    arr->data[index] = NULL;
    arr->size -= 1;
    arr->removed +=1;
    pthread_mutex_unlock(&arr->lock);
}


visible void array_insert(array* arr, const char* value, size_t index){
    pthread_mutex_lock(&arr->lock);
    if (arr->size >= arr->capacity) {
        array_add(arr,NULL);
    }
    if (arr->data[index] == NULL){
        arr->data[index] = (char*)value;
        arr->size++;
        pthread_mutex_unlock(&arr->lock);
        return;
    }
    char* tmp = strdup(arr->data[index]);
    char* tmp2;
    arr->data[index] = strdup(value);
    size_t start = index+1;
    while(start < arr->capacity){
        if(arr->data[start] == NULL){
            arr->data[start] = tmp;
            arr->size+=1;
            pthread_mutex_unlock(&arr->lock);
            return;
        }
        tmp2 = strdup(arr->data[start]);
        arr->data[start] = strdup(tmp);
        tmp = strdup(tmp2);
        start++;
    }
    arr->size += 1;
    pthread_mutex_unlock(&arr->lock);
}

visible void array_sort(array* arr){
    pthread_mutex_lock(&arr->lock);
    char** new_data = (char**)calloc(arr->capacity,sizeof(char*));
    size_t start = 0;
    size_t skip = 0;
    while(start < arr->size+arr->removed+1){
        if(arr->data[start] == NULL){
            start++;
            skip++;
            continue;
        }
        new_data[start-skip]=strdup(arr->data[start]);
        start++;
    }
    csort(new_data, arr->size);
    arr->data = new_data;
    pthread_mutex_unlock(&arr->lock);
}

visible char **array_get(array *arr, size_t* len) {
    if (!arr) {
        return NULL;
    }
    
    pthread_mutex_lock(&arr->lock);
    
    
    // Allocate memory for the return array
    char** ret = malloc((arr->size + 1)*sizeof(char*));
    if (!ret) {
        pthread_mutex_unlock(&arr->lock);
        return NULL; // Handle memory allocation failure
    }

    size_t start = 0;
    size_t ret_index = 0; // Index for ret array
    while (start < arr->size) {
        if (arr->data[start] != NULL) {
            ret[ret_index] = strdup(arr->data[start]);
            debug("item: %s index: %ld len: %ld\n", ret[ret_index], ret_index, arr->size);
            if (!ret[ret_index]) {
                // Handle strdup failure
                // Free previously allocated strings in ret
                for (size_t i = 0; i < ret_index; i++) {
                    free(ret[i]);
                }
                free(ret);
                pthread_mutex_unlock(&arr->lock);
                return NULL; // Return NULL on failure
            }
            ret_index++;
        }
        start++;
    }
    // Set length if len is not NULL
    if (len) {
        *len = ret_index;
    }
    
    pthread_mutex_unlock(&arr->lock);
    return ret; // Caller is responsible for freeing this memory
}

visible size_t array_length(array *arr) {
    return arr->size-arr->removed;
}


visible void array_reverse(array *arr) {
    pthread_mutex_lock(&arr->lock);
    if (arr->size <= 1) {
        pthread_mutex_unlock(&arr->lock);
        return; /* No need to reverse if size is 0 or 1 */
    }

    size_t start = 0;
    size_t tot = arr->size + arr->removed;
    while (start < tot/2) {
        /* Swap elements at start and end indices */
        char *temp = arr->data[start];
        char* temp2 = arr->data[tot-start-1];
        if(temp2 != NULL){
            arr->data[start] = strdup(temp2);
        }
        if(temp != NULL){
            arr->data[tot-start-1] = strdup(temp);
        }else{
            arr->data[tot-start-1] = NULL;
        }

        /* Move towards the center */
        start++;
    }
    pthread_mutex_unlock(&arr->lock);
}

visible void array_unref(array *arr) {
    if (arr == NULL) {
        return; // Nothing to free
    }

    // Free each string in the array
    for (size_t i = 0; i < arr->size; i++) {
        free(arr->data[i]); // Free each string
    }

    // Free the array of strings itself
    free(arr->data);

    // Destroy the mutex if it was initialized
    pthread_mutex_destroy(&arr->lock);

    // Free the array structure
    free(arr);
}

visible void array_clear(array* arr){
    if (arr == NULL) {
        return; // Nothing to free
    }
    array_unref(arr);
    arr = array_new();
}
