#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <utils/jobs.h>

// Example callback function
int example_callback(void* args) {
    int* num = (int*)args;
    printf("Job executed with argument: %d\n", *num);
    return 0;
}

int main() {
    // Create a new job manager
    jobs* job_manager = jobs_new();
    if (job_manager == NULL) {
        fprintf(stderr, "Failed to create job manager\n");
        return EXIT_FAILURE;
    }

    // Add jobs to the job manager
    for (int i = 0; i < 10; i++) {
        int* arg = malloc(sizeof(int));
        *arg = i; // Set the argument for the job
        jobs_add(job_manager, (callback)example_callback, arg, NULL);
    }

    // Run the jobs in the job manager
    jobs_run(job_manager);

    // Release the resources used by the job manager
    jobs_unref(job_manager);

    return EXIT_SUCCESS;
}
