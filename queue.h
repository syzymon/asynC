#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct queue queue_t;
typedef void* queue_item;

int queue_init(queue_t* q);

int queue_push(queue_t* q, queue_item item);

bool queue_empty(queue_t* q);

queue_item queue_poll(queue_t* q);

#endif QUEUE_H
