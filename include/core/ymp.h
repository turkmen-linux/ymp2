#ifndef _ymp_h
#define _ymp_h
/**
 * @file ymp.h
 * @brief Main header file for the libymp library.
 *
 * This library is responsible for managing and executing events through an
 * operation manager. It provides an interface for initializing and
 * interacting with the Ymp structure, which encapsulates the operation
 * management functionality.
 */

#include <core/operations.h>
#include <core/variable.h>
#include <utils/array.h>

/**
 * @struct Ymp
 * @brief Represents the main structure for the Ymp library.
 *
 * The Ymp structure contains a pointer to an OperationManager, which is
 * responsible for handling various operations and events within the
 * library. This structure serves as the primary interface for users
 * to interact with the library's functionality.
 */
typedef struct {
    OperationManager *manager; /**< Pointer to the main `OperationManager` instance */
    VariableManager *variables; /**< Pointer to the main `VariableManager` instance */
    array *errors; /**< Pointer to an array that holds error messages. */
/** @cond */
    void* priv_data; /* Private data. Do not touch! */
/** @endcond */
} Ymp;


/**
 * @brief Initializes a Ymp structure.
 *
 * This function allocates memory for a new Ymp structure and initializes
 * its internal components, including the associated OperationManager.
 * The user is responsible for managing the memory of the returned
 * Ymp structure, including freeing it when it is no longer needed.
 *
 * @return A pointer to the newly created Ymp structure, or NULL if
 *         the initialization fails (e.g., due to memory allocation
 *         failure).
 */
Ymp* ymp_init();

/**
 * @brief Adds a new entry to the Ymp structure.
 *
 * This function adds a new entry identified by the given name to the
 * specified Ymp structure. The additional arguments can be passed
 * through the `args` parameter, which can be used to provide any
 * necessary data for the new entry.
 *
 * @param ymp Pointer to the Ymp structure where the entry will be added.
 * @param name The name of the entry to be added. This should be a
 *             null-terminated string.
 * @param args Pointer to additional arguments that may be needed for
 *             the entry. The interpretation of this pointer depends on
 *             the specific implementation and usage context.
 *
 * @note Ensure that the Ymp structure is properly initialized before
 *       calling this function. The behavior is undefined if the
 *       structure is not valid.
 */
void ymp_add(Ymp* ymp, const char* name, void* args);

/**
 * @brief Executes operations on items in the Ymp queue.
 *
 * This function iterates through the items in the Ymp queue and performs
 * the main operation for each item. If an operation returns a positive
 * value, the iteration is halted, and the return code of the operation
 * is returned.
 *
 * @param ymp A pointer to a Ymp structure that contains the queue and
 *            associated data.
 * @return int Returns the return code of the operation. A positive
 *             value indicates that the operation was successful and
 *             the iteration was stopped. A value of 0 or negative
 *             indicates that all operations were executed without
 *             early termination.
 * @code
 * // example usage:
 * int main(int argc, char** argv){
 *     Ymp *ymp = ymp_new();
 *     ymp_add(ymp, "print", "Hello World\n");
 *     ymp_add(ymp, "print", "How are you?\n");
 *     return ymp_run(ymp);
 * }
 * @endcode
 */

int ymp_run(Ymp* ymp);

#endif
