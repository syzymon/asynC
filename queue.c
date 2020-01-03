#include "queue.h"
#include <stdlib.h>
#include <assert.h>

void queue_init(queue_t *q) {
    q->head = q->tail = NULL;
}

void queue_destroy(queue_t *q) {
    assert(queue_empty(q));
}

bool queue_empty(queue_t *q) {
    return q->head == NULL;
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