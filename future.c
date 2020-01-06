#include "future.h"
#include "threadpool.h"

void handle_future(void *arg, size_t argsz __attribute__((unused))) {
    int err = 0;
    future_t *future = arg;
    future->result = future->callable.function(future->callable.arg,
                                               future->callable.argsz,
                                               &future->result_size);

    P(&future->mutex);
    future->completed = true;
    SIGNAL(&future->wait_for_completed);

    if (future->next != NULL) {
        future_t *mapping = future->next;
        mapping->callable.arg = future->result;
        mapping->callable.argsz = future->result_size;

        P(&mapping->pool->mutex);

        if ((err = _defer(mapping->pool, (runnable_t) {
                .function=handle_future,
                .arg=mapping,
                .argsz=sizeof(future_t)
        }))) {
            V(&mapping->pool->mutex);
            V(&future->mutex);
            return;
        }

        --mapping->pool->pending_maps;
        V(&mapping->pool->mutex);
    }
    V(&future->mutex);
}

int future_init(future_t *future) {
    int err = 0;

    if ((err = pthread_mutex_init(&future->mutex, NULL)) != 0)
        syserr(err, "mutex init failed");
    if ((err = pthread_cond_init(&future->wait_for_completed, NULL))
        != 0)
        syserr(err, "cond init failed");

    future->completed = false;
    future->result = NULL;
    future->result_size = 0;
    future->next = NULL;
    future->pool = NULL;
    return 0;
}

/**
 * Should be called only when pool mutex is acquired.
 */
int _async(thread_pool_t *pool, future_t *future, callable_t callable) {
    int err = 0;

    if ((err = future_init(future)) != 0)
        return err;
    future->callable = callable;
    future->pool = pool;

    _defer(pool, (runnable_t) {
            .arg = future,
            .argsz = sizeof(future_t),
            .function = handle_future
    });
    return err;
}

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    int err = 0;
    CHECK_POOL();
    P(&pool->mutex);
    if (pool->active == false) {
        V(&pool->mutex);
        return -1;
    }
    err = _async(pool, future, callable);
    V(&pool->mutex);
    return err;
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    int err = 0;
    CHECK_POOL();
    P(&pool->mutex);
    if (pool->active == false) {
        V(&pool->mutex);
        return -1;
    }

    P(&from->mutex);

    callable_t to_do = {
            .function = function,
            .arg = from->completed ? from->result : NULL,
            .argsz = from->completed ? from->result_size : 0
    };

    if (from->completed) {
        if ((err = _async(pool, future, to_do)) != 0) {
            V(&pool->mutex);
            V(&from->mutex);
            return err;
        }
        V(&pool->mutex);
    } else {
        ++pool->pending_maps;
        V(&pool->mutex);

        future_init(future);
        future->callable = to_do;
        future->pool = pool;
        from->next = future;
    }
    V(&from->mutex);
    return 0;
}

void *await(future_t *future) {
    int err = 0;
    P(&future->mutex);

    while (!future->completed)
        WAIT(&future->wait_for_completed, &future->mutex);

    V(&future->mutex);

    if ((err = pthread_mutex_destroy(&future->mutex)) != 0)
        syserr(err, "mutex destroy failed");
    if ((err = pthread_cond_destroy(&future->wait_for_completed)) != 0)
        syserr(err, "future cond failed");

    return future->result;
}
