#include "future.h"
#include "threadpool.h"
#include <stdlib.h>

typedef void *(*function_t)(void *);

int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    int err = 0;

    err = pthread_mutex_init(&future->wait_for_result, 0);
    if (err) return err;

    future->result = NULL;




    return err;
}

int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    int err = await(from);
    if(err != 0) return err;
    callable_t from_result = {
            .function = function,
            .arg = from->result,
            .argsz = from->result_size
    };
    return async(pool, future, from_result);
}

void *await(future_t *future) {
    int err = 0;
    P(&future->wait_for_result);
    return future->result;
}
