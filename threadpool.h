#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct runnable {
    void (*function)(void *, size_t);
    void *arg;
    size_t argsz;
} runnable_t;

typedef struct thread_pool {
    size_t num_threads;
    bool active;
    pthread_mutex_t mutex;
    pthread_cond_t wait_for_job;
    struct queue_root *task_queue_ptr;
    pthread_t *threads;
    size_t pending_maps;
    size_t global_index;
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

typedef struct queue_item {
    runnable_t to_do;
    struct queue_item *next;
} queue_item_t;

typedef struct queue_root {
    queue_item_t *head;
    queue_item_t *tail;
} queue_t;

#define syserr(bl, fmt) \
    fprintf(stderr, "ERROR: %s (%d; %s)\n", fmt, bl, strerror(bl)), exit(1)

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

#define CHECK_POOL() \
    if(pool == NULL || !pool->active) \
        return -1

int _defer(struct thread_pool *pool, runnable_t runnable);

#endif
