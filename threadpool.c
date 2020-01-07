#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include "threadpool.h"
#include "vector.h"

void queue_init(queue_t *q) {
    q->head = q->tail = NULL;
}

bool queue_empty(queue_t *q) {
    return q->head == NULL;
}

void queue_destroy(queue_t *q) {
    assert(queue_empty(q));
}

int queue_push(queue_t *q, runnable_t contents) {
    queue_item_t *item = malloc(sizeof(queue_item_t));
    if (item == NULL)
        return -1;

    item->to_do = contents;
    item->next = NULL;
    if (q->head == NULL)
        q->head = q->tail = item;
    else
        q->tail = q->tail->next = item;
    return 0;
}

runnable_t queue_poll(queue_t *queue) {
    assert(!queue_empty(queue));
    runnable_t popped;

    popped = queue->head->to_do;
    queue_item_t *next = queue->head->next;
    free(queue->head);
    queue->head = next;
    if (queue->head == NULL)
        queue->tail = NULL;
    return popped;
}


static vector_t *all_active_pools() {
    static bool initialized = false;
    static vector_t active_pools;
    if (!initialized) {
        vector_init(&active_pools);
        initialized = true;
    }
    return &active_pools;
}

int record_new_pool(thread_pool_t *tp) {
    vector_t *vec = all_active_pools();
    tp->global_index = vector_size(vec);
    return vector_push_back(vec, tp);
}

void erase_inactive_pool(thread_pool_t *tp) {
    vector_t *vec = all_active_pools();
    size_t index_of_erase = tp->global_index;
    vector_erase_at(vec, index_of_erase);
    size_t sz = vector_size(vec);
    if (index_of_erase < sz) {
        thread_pool_t *to_update = vector_at(vec, index_of_erase);
        to_update->global_index = index_of_erase;
    }
}

static void destroy_after_sigint(thread_pool_t *pool) {
    int err = 0;
    pool->active = false;
    if ((err = pthread_cond_broadcast(&pool->wait_for_job)))
        syserr(err, "broadcast failed");
    erase_inactive_pool(pool);

    for (size_t i = 0; i < pool->num_threads; ++i) {
        if ((err = pthread_join(pool->threads[i], NULL)) != 0)
            syserr(err, "join failed");
    }

    assert(pool->pending_maps == 0);
    free(pool->threads);

    if ((err = pthread_mutex_destroy(&pool->mutex)))
        syserr(err, "mutex destroy failed");
    if ((err = pthread_cond_destroy(&pool->wait_for_job)))
        syserr(err, "cond destroy failed");

    queue_destroy(pool->task_queue_ptr);
    free(pool->task_queue_ptr);
}


static void handle_sigint(int no __attribute__((unused))) {
    vector_do_for_each(all_active_pools(),
                       (void (*)(void *)) destroy_after_sigint);
}

void mask_block_sigint(sigset_t *mask_ptr) {
    sigemptyset(mask_ptr);
    sigaddset(mask_ptr, SIGINT);
}

void init_sighandler() {
    int err = 0;
    sigset_t mask;
    mask_block_sigint(&mask);

    if ((err = sigaction(SIGINT, &(struct sigaction) {
            .sa_handler=handle_sigint,
            .sa_mask=mask,
            .sa_flags=0
    }, NULL)) != 0)
        syserr(err, "sighandler init failed");
}

__attribute__((constructor))
static void before_main() {
    init_sighandler();
    all_active_pools(); // initialize vector with pools
}

__attribute__((destructor))
static void after_main() {
    vector_t *v = all_active_pools();
    vector_empty_destroy(v);
}

bool active_or_tasks_pending(thread_pool_t *pool) {
    return pool->active || !queue_empty(pool->task_queue_ptr) ||
           pool->pending_maps > 0;
}

void *thread_worker(void *data) {
    int err = 0;
    sigset_t mask;
    mask_block_sigint(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)))
        syserr(err, "sigmask error");

    thread_pool_t *t = data;

    while (true) {
        P(&t->mutex);
        if (!active_or_tasks_pending(t))
            break;

        while (queue_empty(t->task_queue_ptr) &&
               (t->active == true || t->pending_maps > 0))
            WAIT(&t->wait_for_job, &t->mutex);

        if (!active_or_tasks_pending(t))
            break;
        runnable_t job = queue_poll(t->task_queue_ptr);
        V(&t->mutex);

        job.function(job.arg, job.argsz);

        P(&t->mutex);

        if (!active_or_tasks_pending(t))
            break;
        V(&t->mutex);
    }
    if ((err = pthread_cond_broadcast(&t->wait_for_job)) != 0)
        syserr(err, "broadcast failed");
    V(&t->mutex);
    return (void *) 0;
}

int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
    int err = 0;

    if ((err = pthread_mutex_init(&pool->mutex, NULL)) != 0)
        syserr(err, "mutex init error");

    P(&pool->mutex);

    if ((err = pthread_cond_init(&pool->wait_for_job, NULL)) != 0) {
        syserr(err, "cond init error");
    }

    pool->task_queue_ptr = malloc(sizeof(queue_t));
    if (pool->task_queue_ptr == NULL) return -1;
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

    pool->pending_maps = 0;
    pool->num_threads = num_threads;
    pool->active = true;
    record_new_pool(pool);
    V(&pool->mutex);
    return 0;
}

void thread_pool_destroy(thread_pool_t *pool) {
    int err = 0;
    P(&pool->mutex);
    pool->active = false;
    if ((err = pthread_cond_broadcast(&pool->wait_for_job)))
        syserr(err, "broadcast failed");
    erase_inactive_pool(pool);
    V(&pool->mutex);

    for (size_t i = 0; i < pool->num_threads; ++i) {
        if ((err = pthread_join(pool->threads[i], NULL)) != 0)
            syserr(err, "join failed");
    }

    assert(pool->pending_maps == 0);
    free(pool->threads);

    if ((err = pthread_mutex_destroy(&pool->mutex)))
        syserr(err, "mutex destroy failed");
    if ((err = pthread_cond_destroy(&pool->wait_for_job)))
        syserr(err, "cond destroy failed");

    queue_destroy(pool->task_queue_ptr);
    free(pool->task_queue_ptr);
}

/**
 * Should be called only when pool mutex is acquired.
 */
int _defer(struct thread_pool *pool, runnable_t runnable) {
    int err = 0;
    if ((err = queue_push(pool->task_queue_ptr, runnable)) != 0)
        return err;
    SIGNAL(&pool->wait_for_job);
    return err;
}

int defer(struct thread_pool *pool, runnable_t runnable) {
    int err = 0;
    CHECK_POOL();
    P(&pool->mutex);
    if (!pool->active) {
        V(&pool->mutex);
        return -1;
    }
    err = _defer(pool, runnable);
    V(&pool->mutex);
    return err;
}