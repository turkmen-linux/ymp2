#ifndef _operations_h
#define _operations_h
#include <stddef.h>

/**
 * @file operations.h
 * @brief Header file for operation registration and callback handling.
 */

/**
 * @typedef callback
 * @brief A function pointer type for operations.
 *
 * This type defines a pointer to a function that takes a single argument of type `void*`
 * and returns an integer. It can be used to implement various operations that require
 * callback functionality.
 *
 * @return int The result of the operation, typically indicating success (0) or failure (non-zero).
 * @code
 * // example usage:
 * int test(void* args){
 *     ...
 *    return 0;
 * }
 * int main(int argc, char** argv){
 *     OperationManager *manager = operation_manager_new();
 *     Operation op;
 *     op.call = (callback)test;
 *     op.alias = "test:t:hmm";
 *     op.name = "test";
 *     operation_register(manager, op);
 *     return operation_main(manager, "test", NULL);
 * }
 * @endcode
 */
typedef int (*callback)(void*, ...);

/**
 * @struct Operation
 * @brief Structure representing an operation.
 *
 * This structure holds information about an operation, including a pointer to the
 * callback function and any private data associated with the operation.
 */
typedef struct {
    callback call;      /**< Pointer to the callback function for the operation. */
    char* alias;        /** Operation name alias. <*/
    size_t min_args;    /**< Minimum argument length **/
    char* name;         /**< Name of the operation, used for identification. */
} Operation;

/**
 * @struct OperationManager
 * @brief Structure for managing an array of operations.
 *
 * This structure is used to manage a dynamic array of `Operation` structures.
 * It keeps track of the current number of operations, the capacity of the array,
 * and provides a way to efficiently manage the operations.
 */
typedef struct {
    Operation *operations; /**< Pointer to the array of operations. */
    Operation on_error;    /**< Operation that called if error detected */
    size_t length;         /**< Current number of operations in the array. */
    size_t capacity;       /**< Current capacity of the array. */
/** @cond */
    void* priv_data;       /* Private data. Do not touch! */
/** @endcond */
} OperationManager;

/**
 * @brief Registers a new operation.
 *
 * This function takes an `Operation` structure and registers it for use in the system.
 * The registered operation can be invoked later using the callback function specified
 * in the `Operation` structure.
 *
 * @param manager A pointer to the `OperationManager` instance where the operation will be registered.
 * @param new_op The `Operation` structure to be registered, containing the callback and private data.
 *
 * @note Ensure that the callback function pointed to by `new_op.call` is valid and
 *       properly defined before calling this function.
 *
 */
void operation_register(OperationManager *manager, Operation new_op);

/**
 * @brief Creates a new OperationManager instance.
 *
 * This function allocates memory for a new `OperationManager` and initializes
 * its members. The operations array is initially set to NULL, and the length
 * and capacity are set to zero.
 *
 * @return OperationManager* Pointer to the newly created `OperationManager` instance,
 *         or NULL if memory allocation fails.
 *
 * @note The caller is responsible for freeing the memory allocated for the
 *       `OperationManager` instance using `operation_manager_cleanup` or a similar
 *       function when it is no longer needed.
 */
OperationManager *operation_manager_new();

/**
 * @brief Executes the main operation managed by the specified OperationManager.
 *
 * This function is responsible for initiating and managing the execution of
 * an operation identified by the given name. It utilizes the provided
 * arguments to configure the operation as needed.
 *
 * @param manager A pointer to the `OperationManager` instance that manages
 *                the operations. This must not be NULL.
 * @param name A string representing the name of the operation to be executed.
 *             This should correspond to a valid operation registered with
 *             the `OperationManager`.
 * @param args A pointer to the arguments required for the operation. The
 *             type and structure of this data depend on the specific operation
 *             being executed. This may be NULL if no arguments are needed.
 *
 * @return int Returns 0 on success, or a negative error code on failure.
 *             The specific error codes depend on the implementation and
 *             the nature of the failure encountered during operation execution.
 *
 * @note Ensure that the `OperationManager` is properly initialized before
 *       calling this function. The operation name must be valid and
 *       registered with the manager.
 */
int operation_main(OperationManager *manager, const char* name, void* args);

#endif