#ifndef ASYNC_DEFER_H
#define ASYNC_DEFER_H

#include "threadpool.h"

int _defer(struct thread_pool *pool, runnable_t runnable);

#endif //ASYNC_DEFER_H
