#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;

typedef struct queue_item {
    runnable_t to_do;
    struct queue_item* next;
} queue_item_t;

typedef struct queue_root {
    struct queue_item* head;
    struct queue_item* tail;
} queue_t;

typedef struct thread_pool {
    size_t num_threads;
    bool active;
    pthread_mutex_t mutex;
    pthread_cond_t wait_for_job;
    queue_t *task_queue_ptr;
    pthread_t *threads;
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif
