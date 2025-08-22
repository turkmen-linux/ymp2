#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <core/variable.h>
#include <core/logger.h>
#include <core/ymp.h>

typedef struct {
    char* name;
    char* value;
    bool read_only;
} YmpVariable;

visible VariableManager* variable_manager_new() {
    VariableManager *variables = (VariableManager*)malloc(sizeof(VariableManager));
    if (!variables) {
        return NULL; // Handle memory allocation failure
    }
    variables->capacity = 32; // Initialize with a reasonable capacity
    variables->length = 0;
    variables->priv_data = malloc(sizeof(YmpVariable) * variables->capacity);
    if (!variables->priv_data) {
        free(variables);
        return NULL; // Handle memory allocation failure
    }
    return variables;
}

static void variable_set_value_fn(VariableManager* variables, const char* name, const char* value, bool read_only) {
    if (!variables) {
        printf("Invalid VariableManager\n");
        return;
    }
    YmpVariable* vars = (YmpVariable*)variables->priv_data;

    // Search for existing variable
    for (size_t i = 0; i < variables->length; i++) {
        if (!read_only && vars[i].read_only) {
            continue;
        }
        if (strcmp(name, vars[i].name) == 0) {
            free(vars[i].value); // Free old value
            vars[i].value = strdup(value); // Allocate new memory for the value
            return;
        }
    }

    if (variables->length >= variables->capacity) {
        // Reallocate variable storage
        variables->capacity += 32;
        YmpVariable* new_vars = realloc(variables->priv_data, sizeof(YmpVariable) * variables->capacity);
        if (!new_vars) {
            printf("Memory allocation failed\n");
            return; // Handle memory allocation failure
        }
        variables->priv_data = new_vars;
        vars = new_vars; // Update local pointer
    }

    debug("variable set: %s => %s\n", name, value);
    vars[variables->length].name = strdup(name); // Allocate memory for the name
    vars[variables->length].value = strdup(value); // Allocate memory for the value
    vars[variables->length].read_only = read_only;
    variables->length++;
}

void visible variable_set_value(VariableManager* variables, const char* name, const char* value) {
    variable_set_value_fn(variables, name, value, false);
}

void visible variable_set_value_read_only(VariableManager* variables, const char* name, const char* value) {
    variable_set_value_fn(variables, name, value, true);
}

visible char* variable_get_value(VariableManager* variables, const char* name) {
    if (!variables) {
        printf("Invalid VariableManager\n");
        return "";
    }
    YmpVariable* vars = (YmpVariable*)variables->priv_data;
    debug("variable get: %s\n", name);
    for (size_t i = 0; i < variables->length; i++) {
        if (strcmp(name, vars[i].name) == 0) {
            return vars[i].value; // Return the value if found
        }
    }
    return ""; // Return empty string if not found
}

char* get_value(const char* name) {
    if (!global) {
        global = ymp_init();
        warning("please call ymp_init after use get_value");
    }
    return variable_get_value(global->variables, name);
}

void set_value(const char* name, const char* value) {
    if (!global) {
        global = ymp_init();
        warning("please call ymp_init after use set_value");
    }
    variable_set_value(global->variables, name, value);
}

void set_value_read_only(const char* name, const char* value) {
    if (!global) {
        global = ymp_init();
        warning("please call ymp_init after use set_value_read_only");
    }
    variable_set_value_read_only(global->variables, name, value);
}

