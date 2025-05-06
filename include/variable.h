/**
 * @file variable.h
 * @brief Define string or boolean variables.
*/

#include <stddef.h>

/**
 * @struct VariableManager
 * @brief Variable storage struct.
*/
typedef struct {
/** @cond */
    void* priv_data; /* Private data. Do not touch ! */
    size_t length;
    size_t capacity;
/** @endcond */
} VariableManager;

/**
 * @function variable_manager_new
 * @brief Create a new `VariableManager` instance
*/

VariableManager *variable_manager_new();

extern VariableManager* global_variables;

void variable_set_value(VariableManager* variables, const char* name, const char* value);
char* variable_get_value(VariableManager* variables, const char* name);