#include <stdio.h>
#include <stdlib.h>
#include <utils/string.h>

int main() {
    // Example strings
    char* str1 = "Hello, World!";
    char* str2 = "Hello, World!";
    char* str3 = "Goodbye!";

    // String comparison
    if (iseq(str1, str2)) {
        printf("'%s' is equal to '%s'\n", str1, str2);
    } else {
        printf("'%s' is not equal to '%s'\n", str1, str2);
    }

    // Read file
    char *os_release = readfile("/etc/os-release");
    printf("%s\n", os_release);
    // Read resource
    char* example_txt = readfile(":/example.txt");
    printf("%s\n", example_txt);

    // Count tab characters
    char* data = "Hello\tWorld\t!";
    long tab_count = count_tab(data);
    printf("Number of tab characters in '%s': %ld\n", data, tab_count);

    // Join strings
    char* array[] = {"Hello", "World", "from", "C!", NULL};
    char* joined = join(" ", array);
    printf("Joined string: '%s'\n", joined);
    free(joined); // Remember to free the allocated memory

    // Concatenate strings
    char* concatenated = str_add(str1, str3);
    printf("Concatenated string: '%s'\n", concatenated);
    free(concatenated); // Free the allocated memory

    // Trim whitespace
    char* whitespace_str = "   Trim me!   ";
    char* trimmed = trim(whitespace_str);
    printf("Trimmed string: '%s'\n", trimmed);
    free(trimmed); // Free the allocated memory

    // Integer to string conversion
    int num = 42;
    char* num_str = int_to_string(num);
    printf("Integer to string: '%s'\n", num_str);
    free(num_str); // Free the allocated memory

    // URL encoding and decoding
    char* url = "Hello World!";
    char* encoded = url_encode(url);
    printf("URL encoded: '%s'\n", encoded);

    char* decoded = url_decode(encoded);
    printf("URL decoded: '%s'\n", decoded);

    free(encoded); // Free the allocated memory
    free(decoded); // Free the allocated memory

    // Check if string starts with a prefix
    if (startswith(str1, "Hello")) {
        printf("'%s' starts with 'Hello'\n", str1);
    }

    // Check if string ends with a suffix
    if (endswith(str3, "!")) {
        printf("'%s' ends with '!'\n", str3);
    }

    // Build a formatted string
    char* formatted = build_string("Formatted string: %s %d", "Number", 100);
    printf("%s\n", formatted);
    free(formatted); // Free the allocated memory
    
    const char *data2 = "Hello,World,This,Is,A,Test";
    const char *delimiter = ",";
    
    char **result = split(data2, delimiter);
    
    for (size_t i = 0; result[i]; i++) {
        printf("%ld-> %s\n",i, result[i]);
    }

    // Free the allocated memory
    free(result);

    return 0;
}
