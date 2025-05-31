#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <utils/string.h>
#include <utils/array.h>
#include <core/logger.h>

#define MAX_LINE_LENGTH 1024

visible bool yaml_has_area(const char *data, const char *path) {
    char line[MAX_LINE_LENGTH];
    FILE *stream = fmemopen((void *)data, strlen(data), "r");
    while (fgets(line, sizeof(line), stream)) {
        if (strncmp(line, path, strlen(path)) == 0 && line[strlen(path)] == ':') {
            fclose(stream);
            return true;
        }
    }
    fclose(stream);
    return false;
}

visible char *yaml_get_area(const char *data, const char *path) {
    char line[MAX_LINE_LENGTH];
    bool in_area = false;
    char *area_data = malloc(strlen(data) + 1);
    area_data[0] = '\0';

    FILE *stream = fmemopen((void *)data, strlen(data), "r");
    while (fgets(line, sizeof(line), stream)) {
        if (line[0] != ' ' && strstr(line, ":")) {
            if (in_area) {
                break; // Exit if we reach a new area
            }
            if (strncmp(line, path, strlen(path)) == 0 && line[strlen(path)] == ':') {
                in_area = true;
                continue;
            }
        }
        if (in_area) {
            strcat(area_data, line);
        }
    }
    fclose(stream);
    // shrink memory
    char *area = strdup(trim(area_data));
    free(area_data);
    return area;
}

visible char *yaml_get_value(const char *data, const char *name) {
    char line[MAX_LINE_LENGTH];
    bool in_value = false;
    char *value = malloc(MAX_LINE_LENGTH);
    value[0] = '\0';

    FILE *stream = fmemopen((void *)data, strlen(data), "r");
    if(!stream){
        return NULL;
    }
    while (fgets(line, sizeof(line), stream)) {
        if (strncmp(line, name, strlen(name)) == 0 && line[strlen(name)] == ':') {
            strcpy(value, line + strlen(name) + 1);
            value[strcspn(value, "\n")] = 0; // Remove newline
            in_value = true;
            break;
        }
    }
    fclose(stream);
    return in_value ? strip(value) : NULL;
}

visible char **yaml_get_array(const char *data, const char *name, int *count) {
    char line[MAX_LINE_LENGTH];
    int max = 32;
    char **array = malloc(max * sizeof(char*));
    *count = 0;

    char *area_data = yaml_get_area(data, name);
    if (!area_data) {
        return NULL;
    }

    FILE *stream = fmemopen(area_data, strlen(area_data), "r");
    while (fgets(line, sizeof(line), stream)) {
        if (line[0] == '-') {
            if (*count >= max){
                max += 32;
                array = realloc(array, max * sizeof(char*));
            }
            array[*count] = malloc(strlen(line) + 1);
            strcpy(array[*count], line + 1); // Skip the '-'
            array[*count][strcspn(array[*count], "\n")] = 0; // Remove newline
            (*count)++;
        }
    }
    fclose(stream);
    free(area_data);
    return array;
}

// Function to get the area list
visible char** yaml_get_area_list(const char* fdata, const char* path, int* area_count) {
    int max = 32;
    char** ret = malloc(max * sizeof(char*));
    for (int i = 0; i < max; i++) {
        ret[i] = NULL; // Initialize pointers to NULL
    }

    array * area = array_new();
    char line[MAX_LINE_LENGTH];
    bool e = false;
    *area_count = 0;

    FILE *stream = fmemopen((void *)fdata, strlen(fdata), "r");
    while (fgets(line, sizeof(line), stream)) {
        while(line[strlen(line)-1] == '\n'){
            line[strlen(line)-1] = '\0';
        }
        if(e){
            if(line[0] != ' ') {
                // Flush memory to array
               // Check if we need to resize the array
                if (*area_count >= max) {
                    max += 32; // Increase size by 32
                    ret = realloc(ret, max * sizeof(char*));
                    if (ret == NULL) {
                        fprintf(stderr, "Memory allocation failed\n");
                        return NULL; // Handle allocation failure
                    }
                }
                ret[*area_count] = trim(array_get_string(area));
                debug("%d %s\n",*area_count, line);
                (*area_count)++;
                array_clear(area);
                e = false;
            } else if (strlen(line) > 0) {
                array_add(area,line);
                array_add(area,"\n");
            }
        } else {
            if(line[0] == ' ' || !strstr(line, ":")){
                continue;
            }
            char* name = strtok(line, ":");
            e = (strcmp(name, path) == 0);
            continue;
        }
    }

    // Flush memory for last item
    if (e) {
        // Check if we need to resize the array
        if (*area_count >= max) {
            max += 32; // Increase size by 32
            ret = realloc(ret, max * sizeof(char*));
            if (ret == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return NULL; // Handle allocation failure
            }
        }
        ret[*area_count] = trim(array_get_string(area));
        (*area_count)++;
    }

    return ret;
}
