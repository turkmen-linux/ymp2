#ifndef _string
#define _string
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include <core/logger.h>

#include <utils/string.h>
#include <utils/array.h>

extern char* resource(const char* path);

visible char* readfile(const char *path) {
    if(strlen(path) > 2){
        if(path[0] == ':' && path[1] == '/'){
            return resource(path);
        }
    }
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = malloc(length + 1);
    long read_items = fread(data, 1, length, file);
    if (read_items != length) {
        perror("Failed to read the complete file");
        free(data);
    }
    data[length] = '\0';
    fclose(file);
    return data;
}

visible long count_tab(char* data){
    int cnt = 0;
    while (*data == ' ') {
        cnt++;
        data++;
    }
    return cnt;
}

visible char* join(const char* f, char** array){
    int i = 0;
    int len = 0;
    /* find output size */
    while(array[i]){
        len += strlen(array[i]) + strlen(f);
        i++;
    }
    /* allocate memory */
    char* ret = calloc(len+1, sizeof(char));
    strcpy(ret,"");
    /* copy item len and reset value */
    int cnt = i;
    i = 0;
    /* copy items */
    while(array[i]){
        strcat(ret,array[i]);
        if(i<cnt-1){
            strcat(ret,f);
        }
	i++;
    }
    return ret;
}

visible char* str_add(char* str1, char* str2){
    char* ret = calloc( (strlen(str1)+strlen(str2)+1),sizeof(char) );
    strcpy(ret,str1);
    strcat(ret,str2);
    return ret;
}

visible char* trim(char *content) {
    // Create a copy of the content to modify
    char *trimmed_content = content;
    if (trimmed_content == NULL) {
        return NULL; // Memory allocation failed
    }

    char *line = strtok(trimmed_content, "\n"); // Tokenize the content by new lines
    if (line == NULL) {
        return trimmed_content; // No content to process
    }

    // Determine the number of leading whitespace characters in the first line
    size_t n = count_tab(line);
    
    // Process the first line
    if (strlen(line) > n) {
        memmove(trimmed_content, line + n, strlen(line) - n + 1); // Trim the first line
    } else {
        line[0] = '\0'; // If n is greater than or equal to line length, set line to empty
    }
    size_t cur = strlen(line) - n +1;
    // Process the remaining lines
    while ((line = strtok(NULL, "\n")) != NULL) {
        if (strlen(line) > n) {
            trimmed_content[cur+1] = '\n';
            memmove(trimmed_content+cur+2, line + n, strlen(line) - n); // Trim the line
            cur+=strlen(line) -n+1;
        } else {
            line[0] = '\0'; // Set line to empty if n is greater than or equal to line length
        }
    }
    trimmed_content[cur+1] = '\0';
    return trimmed_content; // Return the trimmed content
}


visible char* int_to_string(int num){
    char *ret = calloc(((sizeof(num) - 1) / 3 + 2), sizeof(char));
    sprintf(ret, "%d", num);
    return ret;
}

/* Function to perform URL decoding */
visible char* url_decode(const char *input) {
    int cnt = 0;
    int i;
    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] == '%'){
           if (isHexDigit(input[i + 1])){
               if (isHexDigit(input[i + 2])) {
                   /* Skip '%', and the next two characters (assuming they are valid hexadecimal digits) */
                   i += 2;
                   cnt++;
                }
            }
        } else {
            cnt++;
        }
    }

    /* +1 for null-terminator */
    char *output = (char *)calloc((cnt + 1), sizeof(char));

    if (output == NULL) {
        perror("Memory allocation failed\n");
        return (char*) input;
    }

    int j = 0;
    for (i = 0; input[i] != '\0'; i++) {
        if (input[i] == '%') {
            if (isHexDigit(input[i + 1])){
                if (isHexDigit(input[i + 2])) {
                    char hex[3] = {input[i + 1], input[i + 2], '\0'};
                    output[j++] = (char)strtol(hex, NULL, 16);
                    /* Skip '%', and the next two characters */
                    i += 2;
                }
            }
        } else {
            output[j++] = input[i];
        }
    }
    /* Null-terminate the output string */
    output[j] = '\0';

    return output;
}

/* Function to perform URL encoding */
visible char* url_encode(const char *input) {
    int cnt = 0;
    int i;
    for (i = 0; input[i] != '\0'; i++) {
        if (!isalnum_c(input[i])) {
            /* Two characters for % and the hexadecimal digit */
            cnt += 2;
        } else {
            cnt++;
        }
    }

    /* +1 for null-terminator */
    char *output = (char *)malloc((cnt + 1) * sizeof(char));

    if (output == NULL) {
        perror( "Memory allocation failed\n");
        return (char*) input;
    }

    int j = 0;
    for (i = 0; input[i] != '\0'; i++) {
        if (isalnum_c(input[i])) {
            output[j++] = input[i];
        } else {
            sprintf(output + j, "%%%02X", (unsigned char)input[i]);
            /* Move to the next position in the output string */
            j += 3;
        }
    }
    /* Null-terminate the output string */
    output[j] = '\0';

    return output;
}


visible char* build_string(char* format, ...) {
    va_list args;
    va_start(args, format);

    /* Determine the size needed for the string */
    int size = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    /* Allocate memory for the string */
    char* result = (char*)malloc(size);
    if (result == NULL) {
        return "";
    }

    /* Format the string */
    va_start(args, format);
    vsnprintf(result, size, format, args);
    va_end(args);

    return result;
}

visible char* str_replace(const char* str, const char* oldSub, const char* newSub) {
    // Calculate lengths
    size_t strLen = strlen(str);
    size_t oldSubLen = strlen(oldSub);
    size_t newSubLen = strlen(newSub);

    // Count occurrences of oldSub in str
    int count = 0;
    const char* temp = str;
    while ((temp = strstr(temp, oldSub)) != NULL) {
        count++;
        temp += oldSubLen; // Move past the last found occurrence
    }

    // Allocate memory for the new string
    size_t newStrLen = strLen + count * (newSubLen - oldSubLen);
    char* newStr = (char*)malloc(newStrLen + 1); // +1 for the null terminator
    if (!newStr) {
        return NULL; // Memory allocation failed
    }

    // Replace occurrences
    char* pos = newStr;
    while (*str) {
        if (strstr(str, oldSub) == str) {
            // Found oldSub, replace it
            strcpy(pos, newSub);
            pos += newSubLen;
            str += oldSubLen;
        } else {
            // Copy the current character
            *pos++ = *str++;
        }
    }
    *pos = '\0'; // Null-terminate the new string

    return newStr;
}

visible char** split(const char* data, const char* f) {
    if (strlen(data) == 0) {
        char** ret = malloc(sizeof(char*));
        ret[0] = NULL;
        return ret;
    }
    array *a = array_new();
    size_t cur=0;
    size_t i=0;
    size_t s=strlen(f);
    for (i=0; data[i];i++){
        if (strncmp(data+i, f,  s) == 0){
            char word[i - cur + 1];
            strncpy(word, &data[cur], i-cur);
            word[i-cur] = '\0';
            array_add(a, word);
            cur=i+s;
        }
    }
    char word[i - cur + 1];
    strncpy(word, &data[cur], i-cur);
    word[i-cur] = '\0';
    array_add(a, word);
    char** ret = array_get(a, &i);
    array_unref(a);
    return ret;
}


visible char* strip(const char* str) {
    if (str == NULL) {
        return NULL; // Handle NULL input
    }

    // Trim leading whitespace
    const char* src = str;
    while (isspace((unsigned char)*src)) {
        src++;
    }

    // Trim trailing whitespace
    const char* end = src + strlen(src) - 1;
    while (end > src && isspace((unsigned char)*end)) {
        end--;
    }

    // Calculate the length of the trimmed string
    size_t length = end - src + 1;

    // Allocate memory for the new string
    char* trimmed = (char*)malloc(length + 1);
    if (trimmed == NULL) {
        return NULL; // Handle memory allocation failure
    }

    // Copy the trimmed string and null-terminate it
    strncpy(trimmed, src, length);
    trimmed[length] = '\0';
    debug("%s len:%d removed:%d\n", trimmed, length, strlen(str) - length);

    return trimmed;
}

#endif
