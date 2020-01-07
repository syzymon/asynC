#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "minunit.h"
#include "threadpool.h"

int tests_run = 0;

#define NROUNDS 10
#define aimiejegotoczterdziesciicztery 44
void task1(void *args, size_t argsz __attribute__((unused)))
{
    int* val = args;
    printf("%u %d\n", (int)pthread_self(), *val);
}


void task2(void *args, size_t argsz __attribute__((unused)))
{
    int* val = args;
    printf("%u %d\n", (int)pthread_self(), *val);
}

void nieinstant(void *args __attribute__((unused)), size_t argsz __attribute__((unused)))
{
    usleep(500000);
}


static void pong_ping(void *args, size_t argsz __attribute__((unused))) {
  sem_t *ping = args;
  sem_t *pong = (sem_t *)args + 1;

  for (int i = 0; i < NROUNDS; ++i) {
    sem_wait(ping);
    sem_post(pong);
  }

  sem_destroy(ping);
}

static char *ping_pong() {

  thread_pool_t pool;
  thread_pool_init(&pool, 1);

  sem_t *pingpong = malloc(sizeof(sem_t) * 2);
  sem_t *ping = pingpong;
  sem_t *pong = pingpong + 1;

  sem_init(ping, 0, 0);
  sem_init(pong, 0, 0);

  defer(&pool, (runnable_t){.function = pong_ping,
                            .arg = pingpong,
                            .argsz = sizeof(sem_t) * 2});

  for (int i = 0; i < NROUNDS; ++i) {
    sem_post(ping);
    sem_wait(pong);
  }

  sem_destroy(pong);
  free(pingpong);

  thread_pool_destroy(&pool);
  return 0;
}

static char* tasks()
{
    thread_pool_t thpool;
    assert(thread_pool_init(&thpool, 4) == 0);
    int val1 = 42;
    int val2 = 100;
    int i;
    for (i=0; i<1000; i++)
    {
        assert(defer(&thpool, (runnable_t){.function = task1,
                .arg = &val1,
                .argsz = sizeof(int)})==0);

        assert(defer(&thpool, (runnable_t){.function = task2,
                .arg = &val2,
                .argsz = sizeof(int)})==0);

    };

    thread_pool_destroy(&thpool);

    return 0;
}

static char* tasks_break()
{
    thread_pool_t thpool;
    thread_pool_init(&thpool, 15);
    int val1 = 999;
    int val2 = 998;
    int i;
    for (i=0; i<10000; i++)
    {
        assert(defer(&thpool, (runnable_t){.function = task1,
                .arg = &val1,
                .argsz = sizeof(int)})==0);

        assert(defer(&thpool, (runnable_t){.function = task2,
                .arg = &val2,
                .argsz = sizeof(int)})==0);
    };

    thread_pool_destroy(&thpool);

    return 0;
}

static char* wpyte_threads_no_tasks()
{
#define duzo 498 //499 + 1 wiecej valgrind nie zrozumie wiec raczej nie bedzie na wiecej testow
    {
        thread_pool_t thpool;
        thread_pool_init(&thpool, duzo);
        int val1 = 999;
        int val2 = 998;
        int i;
        for (i=0; i<0; i++)
        {
            defer(&thpool, (runnable_t){.function = task1,
                    .arg = &val1,
                    .argsz = sizeof(int)});

            defer(&thpool, (runnable_t){.function = task2,
                    .arg = &val2,
                    .argsz = sizeof(int)});
        };

        thread_pool_destroy(&thpool);
    }
    {
        thread_pool_t thpool;
        thread_pool_init(&thpool, duzo);
        int val1 = 999;
        int val2 = 998;
        int i;
        for (i=0; i<0; i++)
        {
            defer(&thpool, (runnable_t){.function = task1,
                    .arg = &val1,
                    .argsz = sizeof(int)});

            defer(&thpool, (runnable_t){.function = task2,
                    .arg = &val2,
                    .argsz = sizeof(int)});
        };

        sleep(3);
        thread_pool_destroy(&thpool);
    }

    return 0;
}

static char* dokladanie()
{
    thread_pool_t thpool;
    thread_pool_init(&thpool, 10);
    int val1 = 997;
    int i;
    for (i=0; i<9999; i++)
    {
        defer(&thpool, (runnable_t){.function = task1,
                .arg = &val1,
                .argsz = sizeof(int)});
    };
    sleep(4);
    val1= aimiejegotoczterdziesciicztery*2;
    for (i=0; i<9999; i++)
    {
        defer(&thpool, (runnable_t){.function = task1,
                .arg = &val1,
                .argsz = sizeof(int)});
    };
    sleep(3);
    val1 = aimiejegotoczterdziesciicztery;
    for (i=0; i<9999; i++)
    {
        defer(&thpool, (runnable_t){.function = task1,
                .arg = &val1,
                .argsz = sizeof(int)});
    };
    thread_pool_destroy(&thpool);
    return 0;
}

static char* dlugietaski()
{
    thread_pool_t thpool;
    thread_pool_init(&thpool, 10);
    int val1 = aimiejegotoczterdziesciicztery;
    int i;
    time_t start,end;
    time(&start);
    for (i=0; i<100; i++)
    {
        defer(&thpool, (runnable_t){.function = nieinstant,
                .arg = &val1,
                .argsz = sizeof(int)});
    };
    thread_pool_destroy(&thpool);
    time(&end);
    double diff = difftime(end, start);
    printf("long tasks test: diff %f\n", diff);
    assert(diff>=4); //oszukany threadpool
    assert(diff<=6); //slaby podzial    //WYKOMENTOWAC NA CZAS VALGRINDA

    return 0;
}

static char* czekam_na_taski()
{
    thread_pool_t thpool;
    thread_pool_init(&thpool, 5);
    sleep(2);
    thread_pool_destroy(&thpool);
    return 0;
}

static char *all_tests() {
    mu_run_test(ping_pong); printf("passed\n");
    mu_run_test(tasks); printf("passed\n");
    mu_run_test(tasks_break); printf("passed\n");
    mu_run_test(wpyte_threads_no_tasks); printf("passed\n");
    mu_run_test(dokladanie); printf("passed\n");
    mu_run_test(dlugietaski); printf("passed\n");
    mu_run_test(czekam_na_taski); printf("passed\n");
  return 0;
}

int main() {
  char *result = all_tests();
  if (result != 0) {
    printf(__FILE__ ": %s\n", result);
  } else {
    printf(__FILE__ ": ALL TESTS PASSED\n");
  }
  printf(__FILE__ " Tests run: %d\n", tests_run);

  return result != 0;
}
