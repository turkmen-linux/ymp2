#ifndef _jobs_h
#define _jobs_h

#include <pthread.h>
#include <core/operations.h>

/**
 * @file jobs.h
 * @brief parallel job control and management
 */

/**
 * @brief Job structure.
 *
 * This struct represents a single job that can be executed by the job manager.
 */
typedef struct {
    callback call; /**< The callback function to execute for the job. */
    void* args;    /**< Arguments to pass to the callback function. */
    void* ctx;     /**< Context for the job, can be used to store additional information. */
    int id;        /**< Unique identifier for the job. */
} job;

/**
 * @brief Job manager structure.
 *
 * This struct manages a collection of jobs, allowing for adding, running,
 * and tracking the status of jobs.
 */
typedef struct {
    job* jobs;              /**< Array of jobs managed by this job manager. */
    int max;                /**< Maximum number of jobs that can be managed. */
    int current;            /**< Current number of jobs in the manager. */
    int parallel;           /**< Number of jobs that can run in parallel. */
    int finished;           /**< Track the number of finished jobs. */
    int total;              /**< Total number of jobs added to the manager. */
    pthread_cond_t cond;    /**< Condition variable for signaling job completion. */
    bool failed;            /**< is Jobs failed */
} jobs;

/**
 * @brief Release the resources used by the job manager.
 *
 * This function frees the memory allocated for the job manager and its jobs.
 *
 * @param j Pointer to the job manager to free.
 */
void jobs_unref(jobs *j);

/**
 * @brief Add a job to the job manager.
 *
 * This function adds a new job to the job manager with the specified callback,
 * context, and arguments.
 *
 * @param j Pointer to the job manager.
 * @param call The callback function to execute for the job.
 * @param ctx Context for the job.
 * @param args Arguments to pass to the callback function.
 * @param ... Additional arguments for the callback function (if needed).
 */
void jobs_add(jobs* j, callback call, void* ctx, void* args, ...);

/**
 * @brief Run the jobs in the job manager.
 *
 * This function executes the jobs in the job manager, respecting the
 * parallelism constraints.
 *
 * @param j Pointer to the job manager.
 */
void jobs_run(jobs* j);

/**
 * @brief Create a new job manager instance.
 *
 * This function allocates and initializes a new job manager.
 *
 * @return A pointer to the newly created job manager, or NULL if the allocation fails.
 */
jobs* jobs_new();

#endif
