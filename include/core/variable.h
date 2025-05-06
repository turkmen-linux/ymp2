#ifndef _variable_h
#define _variable_h
/**
 * @file variable.h
 * @brief Define string or boolean variables.
*/

#include <stddef.h>

/**
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
 * @brief Create a new `VariableManager` instance
*/

VariableManager *variable_manager_new();

/** @cond
* Global variables. Used by internal functions.
*/
extern VariableManager* global_variables;
/** @endcond */

void variable_set_value(VariableManager* variables, const char* name, const char* value);
void variable_set_value_read_only(VariableManager* variables, const char* name, const char* value);
char* variable_get_value(VariableManager* variables, const char* name);

#endif
