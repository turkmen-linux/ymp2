/**
 * @file ymp.h
 * @brief Main header file for the libymp library.
 *
 * This library is responsible for managing and executing events through an
 * operation manager. It provides an interface for initializing and
 * interacting with the Ymp structure, which encapsulates the operation
 * management functionality.
 */

#include <operations.h>

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

