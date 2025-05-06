#include <pthread.h>

#include <operations.h>

typedef struct {
    callback call;
    void* args;
    void* ctx;
    int id;
} job;

typedef struct {
    job* jobs;
    int max;
    int current;
    int parallel;
    int finished; /* Track the number of finished jobs */
    int total;    /* Total number of jobs */
    pthread_cond_t cond;   /* Condition variable for signaling job completion */
} jobs;

#define MAX_JOB 1024*1024

void jobs_unref(jobs *j);
void jobs_add(jobs* j, callback call, void* ctx, void* args, ...);
void jobs_run(jobs* j);
jobs* jobs_new();
