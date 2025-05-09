#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <sys/sysinfo.h>

#include <limits.h>

#include <core/ymp.h>
#include <utils/jobs.h>
#include <core/variable.h>

typedef struct _worker_job {
    jobs* j;
    int id;
} worker_job;

static void* worker_thread(void* arg) {
    worker_job* jb= (worker_job*)arg;
    jobs *j = jb->j;
    int i;
    for (i = jb->id; i < j->total; i+=j->parallel) {
        j->jobs[i].call((void*)j->jobs[i].ctx, (void*)j->jobs[i].args);
        j->finished++;
    }
    return NULL;
}

visible void jobs_unref(jobs *j) {
    free(j->jobs);
    pthread_cond_destroy(&j->cond);
    free(j);
}

visible void jobs_add(jobs* j, callback call, void* ctx, void* args, ...) {
    if (j->total >= j->max) {
        j->max += 32;
        j->jobs = (job*)realloc(j->jobs, sizeof(job)* j->max);
    }
    job new_job;
    new_job.call = call;
    new_job.args = args;
    new_job.ctx = ctx;
    new_job.id = j->total;
    j->jobs[j->total++] = new_job;
    j->current++;
    pthread_cond_signal(&j->cond);
}

visible void jobs_run(jobs* j) {
    pthread_t* threads = (pthread_t*)calloc(j->parallel, sizeof(pthread_t));
    worker_job *jb[j->parallel];
    int i;
    for (i = 0; i < j->parallel; ++i) {
        jb[i] = (worker_job*)calloc(1,sizeof(worker_job));
        jb[i]->j = j;
        jb[i]->id = i;
        pthread_create(&threads[i], NULL, worker_thread, (void*)jb[i]);
    }
    for (i = 0; i < j->parallel; ++i) {
        pthread_join(threads[i], NULL);
    }
    for (i = 0; i < j->parallel; ++i) {
        free(jb[i]);
    }
    free(threads);
}

visible jobs* jobs_new() {
    jobs* j = (jobs*)malloc(sizeof(jobs));
    j->max = 32;
    j->current = 0;
    j->finished = 0;
    j->total = 0;
    j->parallel = get_nprocs_conf();
    j->jobs = (job*)malloc(j->max * sizeof(job));
    pthread_cond_init(&j->cond, NULL);
    return j;
}
