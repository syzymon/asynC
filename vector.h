#ifndef ASYNC_VECTOR_H
#define ASYNC_VECTOR_H

#include <stdlib.h>
#include <assert.h>

#define INIT_CAP 16

typedef struct ptr_vector {
    void **array;
    size_t size;
    size_t cap;
} vector_t;

int vector_init(vector_t *vec) {
    vec->size = 0;
    vec->cap = INIT_CAP;
    vec->array = malloc(vec->cap * sizeof(void *));
    return vec->array == NULL ? -1 : 0;
}

void vector_empty_destroy(vector_t *vec) {
    assert(vec->size == 0);
    free(vec->array);
}

size_t vector_size(vector_t *vec) {
    return vec->size;
}

void *vector_at(vector_t *vec, size_t idx) {
    assert(idx < vec->size);
    return vec->array[idx];
}

int vector_push_back(vector_t *vec, void *elem) {
    if (vec->size == vec->cap) {
        vec->cap *= 2;
        vec->array = realloc(vec->array, vec->cap * sizeof(void *));
        if (vec->array == NULL) return -1;
    }
    vec->array[vec->size++] = elem;
    return 0;
}

void vector_erase_at(vector_t *vec, size_t idx) {
    assert(idx < vec->size);
    vec->array[idx] = vec->array[--vec->size];
}

void vector_do_for_each(vector_t *vec, void (*action)(void *)) {
    for (size_t i = 0; i < vec->size; ++i) {
        action(vec->array[i]);
    }
}

#endif //ASYNC_VECTOR_H
