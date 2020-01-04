#include "defer.h"
#include "queue.h"
#include "err.h"

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
