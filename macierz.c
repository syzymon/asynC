#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "threadpool.h"

#define N_THREADS 4
#define USEC_IN_MS 1000

typedef struct task {
    int val;
    int64_t* sum_to_write;
    size_t ttw_ms;
    pthread_mutex_t* mtx;
} task_t;


int64_t* sums;

void calculate_cell(void* arg, size_t argsz __attribute__((unused))) {
    int err = 0;
    task_t* task_ptr = arg;

    usleep(USEC_IN_MS * task_ptr->ttw_ms);
    if((err = pthread_mutex_lock(task_ptr->mtx) != 0))
        puts("mutex lock error"), exit(err);

    *task_ptr->sum_to_write += task_ptr->val;

    if((err = pthread_mutex_unlock(task_ptr->mtx)) != 0)
        puts("mutex unlock error"), exit(err);
}

int main(void) {
    size_t k, n;
    scanf("%zu%zu", &k, &n);
    size_t tasks_count = k * n;

    task_t* tasks = malloc(tasks_count * sizeof(task_t));
    sums = calloc(k, sizeof(int64_t));
    pthread_mutex_t* mutices = malloc(k * sizeof(pthread_mutex_t));

    if(tasks == NULL || sums == NULL || mutices == NULL) {
        if(tasks != NULL)
            free(tasks);
        if(sums != NULL)
            free(sums);
        if(mutices != NULL)
            free(mutices);
        puts("out of memory");
        return 0;
    }

    for(size_t i = 0; i < k; ++i)
        pthread_mutex_init(&mutices[i], NULL);

    thread_pool_t tp;
    thread_pool_init(&tp, N_THREADS);


    for(size_t i = 0, task_no = 0; i < k; ++i) {
        for(size_t j = 0; j < n; ++j, ++task_no) {
            int v;
            size_t time_ms;
            scanf("%d%zu", &v, &time_ms);
            tasks[task_no].val = v;
            tasks[task_no].ttw_ms = time_ms;
            tasks[task_no].sum_to_write = &sums[i];
            tasks[task_no].mtx = &mutices[i];
            defer(&tp, (runnable_t) {
                    .arg=&tasks[task_no],
                    .argsz=sizeof(task_t),
                    .function=calculate_cell
            });
        }
    }

    thread_pool_destroy(&tp);
    for(size_t i = 0; i < k; ++i)
        printf("%ld\n", sums[i]);

    for(size_t i = 0; i < k; ++i)
        pthread_mutex_destroy(&mutices[i]);
    free(mutices);
    free(tasks);
    free(sums);
    return 0;
}