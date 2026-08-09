/**
 * @file sandbox.h
 * @brief Namespace sandboxing for build isolation.
 */

#ifndef _sandbox_h
#define _sandbox_h

#include <sched.h>
#include <stdbool.h>
#include <sys/types.h>

#include <utils/array.h>

/**
 * @struct sandbox_handle_t
 * @brief Holds the sandbox configuration.
 */
typedef struct {
    int flags;          /* unshare flags */
    char *hostname;     /* hostname inside the sandbox */
    uid_t uid;          /* host uid mapped to root */
    gid_t gid;          /* host gid mapped to root */
    array *binds;       /* registered mounts as "src target" strings */
    bool network;       /* share the host network */
} sandbox_handle_t;

/**
 * @brief Creates a sandbox handle with default settings.
 */
sandbox_handle_t *sandbox_new();

/**
 * @brief Sets the hostname used inside the sandbox.
 */
void sandbox_configure_hostname(sandbox_handle_t *sandbox, const char *hostname);

/**
 * @brief Sets the unshare flags.
 */
void sandbox_configure_flags(sandbox_handle_t *sandbox, int flags);

/**
 * @brief Sets the host uid/gid mapped to root inside the sandbox.
 */
void sandbox_configure_user(sandbox_handle_t *sandbox, uid_t uid, gid_t gid);

/**
 * @brief Applies the configuration and enters the sandbox.
 *
 * Creates the namespaces, sets the hostname and writes the id mappings.
 */
void sandbox_apply(sandbox_handle_t *sandbox);

/**
 * @brief Frees a sandbox handle.
 */
void sandbox_unref(sandbox_handle_t *sandbox);

/**
 * @brief Registers a mount into the sandbox.
 *
 * A "tmpfs" source is mounted as a fresh tmpfs, anything else is bind mounted.
 * The mount is applied later by sandbox_apply().
 */
void sandbox_configure_bind(sandbox_handle_t *sandbox, const char *src, const char *target);

/**
 * @brief Enables network access in the sandbox.
 *
 * When enabled, the sandbox shares the host network instead of
 * isolating a private network namespace.
 */
void sandbox_configure_network(sandbox_handle_t *sandbox, bool enabled);

/**
 * @brief Enters the sandbox with the default configuration.
 */
void sandbox();

#endif