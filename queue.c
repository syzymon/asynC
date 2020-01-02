#include "queue.h"
#include <stdlib.h>
#include <assert.h>

bool queue_empty(queue_t *q) {
    return 0;
}

queue_item queue_poll(queue_t *q) {
    assert(queue_empty(q));
    return NULL;
}
