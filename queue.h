#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef void* queue_content_t;

typedef struct queue_item {
    queue_content_t contents;
    struct queue_item* next;
} queue_item_t;

typedef struct queue_root {
    struct queue_item* head;
    struct queue_item* tail;
} queue_t;

void queue_init(queue_t* q);

void queue_destroy(queue_t* q);

int queue_push(queue_t* q, queue_content_t item);

bool queue_empty(queue_t* q);

queue_content_t queue_poll(queue_t* q);

#endif //QUEUE_H
