/**
 * @file sandbox.h
 * @brief Namespace sandboxing for build isolation.
 */

#ifndef _sandbox_h
#define _sandbox_h

#define UNSHARE_FLAGS (CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWUSER | CLONE_NEWNET)

/**
 * @brief Enters a sandboxed environment using Linux namespaces.
 *
 * This function sets up mount, UTS, user, and network namespaces
 * to isolate the build process from the host system.
 */
void sandbox();

#endif