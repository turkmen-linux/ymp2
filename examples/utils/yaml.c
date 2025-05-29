#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <utils/yaml.h>

int main() {
    const char *data =
        "area1:\n"
        "  - item1\n"
        "  - item2\n"
        "area2:\n"
        "  - item3\n"
        "  - item4\n"
        "value1: some_value\n"
        "value2: another_value\n";

    // Check if an area exists
    const char *path = "area1";
    if (yaml_has_area(data, path)) {
        printf("Area '%s' exists.\n", path);
    } else {
        printf("Area '%s' does not exist.\n", path);
    }

    // Get the area data
    char *area_data = yaml_get_area(data, path);
    if (area_data) {
        printf("Data in '%s':\n%s", path, area_data);
        free(area_data); // Don't forget to free the allocated memory
    }

    // Get a specific value
    const char *value_name = "value1";
    char *value = yaml_get_value(data, value_name);
    if (value) {
        printf("Value of '%s': %s\n", value_name, value);
    }

    // Get an array of items from an area
    int count;
    char **array = yaml_get_array(data, "area1", &count);
    if (array) {
        printf("Items in 'area1':\n");
        for (int i = 0; i < count; i++) {
            printf("  %s\n", array[i]);
            free(array[i]); // Free each item
        }
        free(array); // Free the array itself
    }

    // Get a list of areas
    int area_count;
    char **area_list = yaml_get_area_list(data, "area1", &area_count);
    if (area_list) {
        printf("Areas found: %d\n", area_count);
        for (int i = 0; i < area_count; i++) {
            printf("  %s\n", area_list[i]);
            free(area_list[i]); // Free each area
        }
        free(area_list); // Free the area list
    }

    return 0;
}
