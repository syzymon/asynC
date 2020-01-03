#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include "threadpool.h"

void queue_init(queue_t *);

void queue_destroy(queue_t *);

int queue_push(queue_t *, runnable_t);

bool queue_empty(queue_t *);

runnable_t queue_poll(queue_t *);

#endif //QUEUE_H
