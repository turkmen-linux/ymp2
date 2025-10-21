#ifndef _variable_h
#define _variable_h
/**
 * @file variable.h
 * @brief Define string or boolean variables.
 */

/** @cond **/
#ifndef invisible
#define invisible ;//
#endif

#include <stddef.h>
/** @endcond */

/**
 * @brief Variable storage struct.
 *
 * This struct is used to manage a collection of variables, providing
 * functionality to set and get variable values.
 */
typedef struct {
    /** @cond */
    void* priv_data; /**< Private data. Do not touch! */
    size_t length;   /**< Current number of variables stored. */
    size_t capacity; /**< Maximum number of variables that can be stored. */
    /** @endcond */
} VariableManager;

/**
 * @brief Create a new `VariableManager` instance.
 *
 * This function allocates and initializes a new `VariableManager`.
 *
 * @return A pointer to the newly created `VariableManager`, or NULL if
 *         the allocation fails.
 */
VariableManager *variable_manager_new();
void variable_manager_unref(VariableManager *variables);

/**
 * @brief Set the value of a variable.
 *
 * This function sets the value of a variable identified by its name.
 * If the variable does not exist, it will be created.
 *
 * @param variables Pointer to the `VariableManager` instance.
 * @param name The name of the variable to set.
 * @param value The value to assign to the variable.
 */
void variable_set_value(VariableManager* variables, const char* name, const char* value);
/** @cond **/
invisible void set_value(const char* name, const char* value);
#define set_bool(A, B) set_value(A, B ? "true" : "false")
/** @endcond */
/**
 * @brief Set the value of a read-only variable.
 *
 * This function sets the value of a variable identified by its name
 * and marks it as read-only. The value cannot be changed after this
 * function is called.
 *
 * @param variables Pointer to the `VariableManager` instance.
 * @param name The name of the variable to set.
 * @param value The value to assign to the variable.
 */
void variable_set_value_read_only(VariableManager* variables, const char* name, const char* value);
/** @cond **/
invisible void set_value_read_only(const char* name, const char* value);
/** @endcond */

/**
 * @brief Get the value of a variable.
 *
 * This function retrieves the value of a variable identified by its name.
 *
 * @param variables Pointer to the `VariableManager` instance.
 * @param name The name of the variable to retrieve.
 * @return A pointer to the value of the variable, or NULL if the variable
 *         does not exist.
 */
char* variable_get_value(VariableManager* variables, const char* name);
/** @cond **/
invisible char* get_value(const char* name);
#define get_bool(A) (strcmp(get_value(A), "true") == 0)
/** @endcond */

#endif
