#include "threadpool.h"
#include <stdlib.h>
#include <string.h>

typedef struct job {
    void *to_do;
    void (*execute)(void *);
} job_t;

void execute_runnable(void *arg) {
    runnable_t *to_do = arg;
    to_do->function(to_do->arg, to_do->argsz);
}

#define P(mutex_ptr) \
    if ((err = pthread_mutex_lock(mutex_ptr)) != 0) \
        syserr (err, "lock failed") \

#define V(mutex_ptr) \
    if ((err = pthread_mutex_unlock(mutex_ptr)) != 0) \
        syserr (err, "unlock failed") \

#define WAIT(cond_ptr, mutex_ptr) \
    if ((err = pthread_cond_wait(cond_ptr, mutex_ptr)) != 0) \
        syserr (err, "wait failed") \

#define SIGNAL(cond_ptr) \
    if ((err = pthread_cond_signal(cond_ptr)) != 0) \
        syserr (err, "wait failed") \


void *thread_worker(void *data) {
    int err = 0;
    thread_pool_t *t = data;
    bool active_or_tasks_pending = true;
    do {
        P(&t->mutex);

        while (queue_empty(t->task_queue_ptr) && t->active == true)
            WAIT(&t->wait_for_job, &t->mutex);

        if(queue_empty(t->task_queue_ptr) && t->active == false) {
            V(&t->mutex);
            break;
        }

        job_t *job = queue_poll(t->task_queue_ptr);
        V(&t->mutex);

        job->execute(job->to_do);

        // cleanup
        // TODO: assumption - only one-level cleanup here - future.h
        free(job->to_do);
        free(job);

        P(&t->mutex);
        active_or_tasks_pending = t->active || !queue_empty(t->task_queue_ptr);
        if(!active_or_tasks_pending)
            if((err = pthread_cond_broadcast(&t->wait_for_job)) != 0)
                syserr(err, "broadcast failed");
        V(&t->mutex);
    } while (active_or_tasks_pending);
    return (void *) 0;
}

void set_sigint_handler() {

}

static pthread_once_t sigint_initialized = PTHREAD_ONCE_INIT;

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
    int err = 0;

    if ((err = pthread_once(&sigint_initialized, set_sigint_handler)) != 0)
        syserr(err, "signal handling error");

    if ((err = pthread_mutex_init(&pool->mutex, NULL)) != 0)
        syserr(err, "mutex init error");

    P(&pool->mutex);

    if ((err = pthread_cond_init(&pool->wait_for_job, NULL)) != 0) {
        syserr(err, "cond init error");
    }

    pool->task_queue_ptr = malloc(sizeof(queue_t));
    if(pool->task_queue_ptr == NULL) return -1;
    queue_init(pool->task_queue_ptr);

    pool->threads = malloc(num_threads * sizeof(pthread_t));
    if (pool->threads == NULL) return -1;

    pthread_attr_t attr;
    if ((err = pthread_attr_init(&attr)) != 0)
        syserr(err, "attr_init failed");
    if ((err = pthread_attr_setdetachstate(
            &attr,
            PTHREAD_CREATE_JOINABLE)) != 0)
        syserr(err, "attr_setdetachstate failed");

    for (size_t i = 0; i < num_threads; ++i) {
        if ((err = pthread_create(&pool->threads[i], &attr, thread_worker,
                                  pool)) != 0)
            syserr(err, "create failed");
    }

    pool->num_threads = num_threads;
    pool->active = true;
    V(&pool->mutex);
    return 0;
}

void thread_pool_destroy(struct thread_pool *pool) {
    int err = 0;
    P(&pool->mutex);
    pool->active = false;
    if((err = pthread_cond_broadcast(&pool->wait_for_job)))
        syserr(err, "broadcast failed");
    V(&pool->mutex);

    for(size_t i = 0; i < pool->num_threads; ++i) {
        if ((err = pthread_join(pool->threads[i], NULL)) != 0)
            syserr(err, "join failed");
    }
    // cleanup
    free(pool->threads);

    if((err = pthread_mutex_destroy(&pool->mutex)))
        syserr(err, "mutex destroy failed");
    if((err = pthread_cond_destroy(&pool->wait_for_job)))
        syserr(err, "cond destroy failed");

    queue_destroy(pool->task_queue_ptr);
    free(pool->task_queue_ptr);
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    int err = 0;
    P(&pool->mutex);

    job_t *job = malloc(sizeof(job_t));
    if (job == NULL) return -1;
    job->to_do = malloc(sizeof(runnable_t));
    if (job->to_do == NULL) return -1;

    memcpy(job->to_do, &runnable, sizeof(runnable_t));
    job->execute = execute_runnable;

    if ((err = queue_push(pool->task_queue_ptr, job)) != 0)
        return err;

    // TODO: order of these 2
    SIGNAL(&pool->wait_for_job);
    V(&pool->mutex);
    return err;
}
