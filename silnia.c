#include <stdio.h>
#include <stdint.h>
#include "future.h"

#define N_THREADS 3

typedef struct partial_result {
    uint64_t res;
    uint64_t multiply_by;
} arg_t;

static void *multiply(void *arg, size_t argsz __attribute__((unused)),
                      size_t *retsz __attribute__((unused))) {
    arg_t *input_values = (arg_t *) arg;
    input_values->res *= input_values->multiply_by;
    input_values->multiply_by += N_THREADS;
    return input_values;
}

int main(void) {
    uint64_t n;
    scanf("%lu", &n);
    thread_pool_t tp;
    thread_pool_init(&tp, N_THREADS);

    future_t futures[n];
    arg_t args[N_THREADS];
    for (size_t beg = 1; beg <= n && beg <= N_THREADS; ++beg) {
        args[beg - 1] = (arg_t) {
                .res = 1,
                .multiply_by = beg,
        };
        async(&tp, &futures[beg - 1], (callable_t) {
                .function=multiply,
                .arg=&args[beg - 1],
                .argsz=sizeof(arg_t)
        });

        for (size_t k = beg + N_THREADS; k <= n; k += N_THREADS) {
            map(&tp,
                &futures[k - 1],
                &futures[k - N_THREADS - 1],
                multiply);
        }
    }

    uint64_t result = 1;
    for(int64_t i = n - 1; i >= 0 && i >= (int64_t)(n - N_THREADS); --i) {
        result *= *(uint64_t *) await(&futures[i]);
    }
    printf("%lu\n", result);
    thread_pool_destroy(&tp);
}
