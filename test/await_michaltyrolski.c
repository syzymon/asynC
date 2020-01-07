#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <err.h>
#include <signal.h>

#include "future.h"
#include "minunit.h"
#define aimiejegotoczterdziesciicztery 44

int tests_run = 0;
const float eps = 0.01f;

void basictask(void *args, size_t argsz __attribute__((unused)))
{
    int* val = args;
    (void)val;
}

void basictaskeps(void *args, size_t argsz __attribute__((unused)))
{
    int* val = args;
    usleep(10*1000);
    (void)val;
}

void longbasictask(void *args, size_t argsz __attribute__((unused)))
{
    int* val = args;
    usleep(500 * 1000);
    (void)val;
}

static void *squared(void *arg, size_t argsz __attribute__((unused)),
                     size_t *retsz __attribute__((unused)))
{
  int n = *(int *)arg;
  int *ret = malloc(sizeof(int));
  *ret = n * n;
  return ret;
}

static void *squared_long(void *arg, size_t argsz __attribute__((unused)),
                     size_t *retsz __attribute__((unused)))
{
    usleep(50000);
    int n = *(int *)arg;
    int *ret = malloc(sizeof(int));
    *ret = n * n;
    return ret;
}

static void *add_long(void *arg, size_t argsz __attribute__((unused)),
                          size_t *retsz __attribute__((unused)))
{
    int n = *(int *)arg;
    int *ret = malloc(sizeof(int));
    *ret = n+3;
    usleep(100000);
    return ret;
}

static void *add_longmickiewiczwielkimpoetabyl(void *arg, size_t argsz __attribute__((unused)),
                      size_t *retsz __attribute__((unused)))
{
    int n = *(int *)arg;
    int *ret = malloc(sizeof(int));
    *ret = n+aimiejegotoczterdziesciicztery;
    usleep(100000);
    return ret;
}

static void *add(void *arg, size_t argsz __attribute__((unused)),
                      size_t *retsz __attribute__((unused)))
{
    int n = *(int *)arg;
    int *ret = malloc(sizeof(int));
    *ret = n+3;
    return ret;
}

static void *add_mickiewiczwielkimpoetabyl(void *arg, size_t argsz __attribute__((unused)),
                                               size_t *retsz __attribute__((unused)))
{
    int n = *(int *)arg;
    int *ret = malloc(sizeof(int));
    *ret = n+aimiejegotoczterdziesciicztery;
    return ret;
}

static void *verylongadd10(void *arg, size_t argsz __attribute__((unused)),
                 size_t *retsz __attribute__((unused)))
{
    int n = *(int *)arg;
    int *ret = malloc(sizeof(int));
    *ret = n+10;
    sleep(2);
    return ret;
}

static char *test_await_simple()
{
  thread_pool_t pool;
  thread_pool_init(&pool, 1);

    {
        future_t future;
        int n = 16;
        assert(async(&pool, &future,
              (callable_t){.function = squared, .arg = &n, .argsz = sizeof(int)})==0);

        int *m = await(&future);
        assert(*m == 256);
        free(m);
    }

  for(int j = 16; j < 20; j++)
  {
      future_t future;
      int n = j;
      assert(async(&pool, &future,
            (callable_t){.function = squared, .arg = &n, .argsz = sizeof(int)})==0);
      int *m = await(&future);
      assert(*m == j*j);
      free(m);
  }
    thread_pool_destroy(&pool);
    thread_pool_init(&pool, 10);

    for(int j = 16; j < 20; j++)
    {
        future_t future;
        int n = j;
        assert(async(&pool, &future,
              (callable_t){.function = squared, .arg = &n, .argsz = sizeof(int)})==0);
        int *m = await(&future);
        assert(*m == j*j);
        free(m);
    }

  thread_pool_destroy(&pool);
  return 0;
}

static char *test_wielokrotne()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 4);
    future_t future;

    for(int j = 16; j < 100; j++)
    {
        int n = j;
        assert(async(&pool, &future,
              (callable_t){.function = squared, .arg = &n, .argsz = sizeof(int)})==0);
        int *m = await(&future);
        assert(*m == j*j);
        free(m);
    }


    thread_pool_destroy(&pool);
    return 0;
}

static char *test_dlugie()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 4);
    future_t future;

    for(int j = 16; j < 100; j++)
    {
        int n = j;
        assert(async(&pool, &future,
              (callable_t){.function = squared_long, .arg = &n, .argsz = sizeof(int)})==0);
        int *m = await(&future);
        assert(*m == j*j);
        free(m);
    }


    thread_pool_destroy(&pool);
    return 0;
}

static char* test_map_basic()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 4)==0);
    future_t future;
    future_t from;
    int n = 4;
    int err;
    err = async(&pool, &from,
          (callable_t){.function = squared, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    sleep(1);
    err = map(&pool, &future, &from, squared);
    assert(err == 0);
    int* val = await(&future);
    int* val2 = await(&from);
    assert(*val == 256);
    free(val);
    free(val2);
    thread_pool_destroy(&pool);
    return 0;
}

static char* test_map_waiting()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 4);
    future_t future;
    future_t from;
    int n = 4;
    int err;
    err = async(&pool, &from,
                (callable_t){.function = squared_long, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    time_t start,end;
    time(&start);
    err = map(&pool, &future, &from, squared);
    time(&end);
    double diff = difftime(end, start);
    (void)diff;
    //assert(diff < eps);
    assert(err == 0);
    int* val = await(&future);
    assert(*val == 256);
    int* val2 = await(&from);
    thread_pool_destroy(&pool);
    free(val);
    free(val2);
    return 0;
}

static char* test_multimap()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 1);
    future_t* futures = malloc(sizeof(future_t) * 100);
    int n = 3;
    int err;
    err = async(&pool, &futures[0],
                (callable_t){.function = add_long, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    time_t start,end;
    for(int j = 1; j < 100; j++)
    {
        time(&start);
        err = map(&pool, &futures[j], &futures[j-1], add_long);
        time(&end);
        //assert(difftime(end, start) < eps);
        assert(err == 0);
    }
    assert(err == 0);
    int* val = await(&futures[99]);
    assert(*val == 303);
    thread_pool_destroy(&pool);

    free(val);
    for(int j = 0; j < 99; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }
    free(futures);
    return 0;
}

static char* test_multimapmieszany()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 10);
    future_t* futures = malloc(sizeof(future_t) * 100);
    int n = 7;
    int err;
    err = async(&pool, &futures[0],
                (callable_t){.function = add_long, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    time_t start,end;
    for(int j = 1; j < 100; j++)
    {
        time(&start);
        err = map(&pool, &futures[j], &futures[j-1], j%2 ? add_long : add_longmickiewiczwielkimpoetabyl);
        time(&end);
//        assert(difftime(end, start) < eps);
        assert(err == 0);
    }
    assert(err == 0);
    int* val = await(&futures[99]);
    assert(*val == 2316);
    for(int j = 0; j < 99; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }
    thread_pool_destroy(&pool);
    free(futures);
    free(val);
    return 0;
}

static char* test_multimapszybki()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 10);
    future_t* futures = malloc(sizeof(future_t) * 100);
    int n = 3;
    int err;
    err = async(&pool, &futures[0],
                (callable_t){.function = add, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    time_t start,end;
    for(int j = 1; j < 100; j++)
    {
        time(&start);
        err = map(&pool, &futures[j], &futures[j-1], add);
        time(&end);
        //assert(difftime(end, start) < eps);
        assert(err == 0);
    }
    assert(err == 0);
    int* val = await(&futures[99]);
    assert(*val == 303);
    for(int j = 0; j < 99; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }
    thread_pool_destroy(&pool);
    free(val);
    free(futures);
    return 0;
}

static char* test_multimapmieszanyszybki()
{
    thread_pool_t pool;
    thread_pool_init(&pool, 10);
    future_t* futures = malloc(sizeof(future_t) * 100);
    int n = 7;
    int err;
    err = async(&pool, &futures[0],
                (callable_t){.function = add, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    time_t start,end;
    for(int j = 1; j < 100; j++)
    {
        time(&start);
        err = map(&pool, &futures[j], &futures[j-1], j%2 ? add : add_mickiewiczwielkimpoetabyl);
        time(&end);
       // assert(difftime(end, start) < eps);
        assert(err == 0);
    }
    assert(err == 0);
    int* val = await(&futures[99]);
    assert(*val == 2316);
    for(int j = 0; j < 99; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }
    thread_pool_destroy(&pool);
    free(futures);
    free(val);
    return 0;
}

static char* test_sigint1()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 3) == 0);
    future_t* futures = malloc(sizeof(future_t) * 100);
    int n = 7;
    int err;
    err = async(&pool, &futures[0],
                (callable_t){.function = add_long, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    time_t start,end;
    for(int j = 1; j < 100; j++)
    {
        time(&start);
        err = map(&pool, &futures[j], &futures[j-1], j%2 ? add_long : add_longmickiewiczwielkimpoetabyl);
        time(&end);
        //assert(difftime(end, start) < eps);
        assert(err == 0);
    }

//    printf("killllll\n");
    raise(SIGINT);
//    printf("po killu\n");

    assert(err == 0);
    int* val = await(&futures[99]);
    assert(*val == 2316);
    for(int j = 0; j < 99; j++) {
        int* v = await(&futures[j]);
        free(v);
    }

  //  printf("destroy\n");

//    thread_pool_destroy(&pool);
    free(futures);
    free(val);
    return 0;
}


static char* test_sigint2()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 1) == 0);
    //printf("pudzian1\n");
    future_t* futures = malloc(sizeof(future_t) * 100);
    int n = 7;
    int err;
    err = async(&pool, &futures[0],
                (callable_t){.function = add_long, .arg = &n, .argsz = sizeof(int)});
    assert(err == 0);
    int tb = aimiejegotoczterdziesciicztery;
    for(int j = 0; j < 10; j++)
    {
        assert(defer(&pool, (runnable_t){.function = basictask,
                .arg = &tb,
                .argsz = sizeof(int)})== 0);
    }
    time_t start,end;
    for(int j = 1; j < 100; j++)
    {
        time(&start);
        err = map(&pool, &futures[j], &futures[j-1], j%2 ? add_long : add_longmickiewiczwielkimpoetabyl);
        time(&end);
       // assert(difftime(end, start) < eps);
        assert(err == 0);
    }
    raise(SIGINT);

    for(int j = 0; j < 10; j++)
    {
        assert(defer(&pool, (runnable_t){.function = basictask,
                .arg = &tb,
                .argsz = sizeof(int)}) != 0);
    }


    assert(err == 0);
    int* val = await(&futures[99]);
    assert(*val == 2316);
    for(int j = 0; j < 99; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }
//    thread_pool_destroy(&pool);
    free(futures);
    free(val);
    return 0;
}

static char* test_sigint3()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 100) == 0);
    raise(SIGINT);
    return 0;
}

static char* test_sigint4()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 100) == 0);
    sleep(1);
    raise(SIGINT);
    return 0;
}

static char* test_sigint5()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 4) == 0);
#define testsigint5int 100
    int tab[testsigint5int];
    for(int i = 0; i < testsigint5int; i++) tab[i] = i;

    for(int i = 0; i < testsigint5int; i++)
    assert(defer(&pool, (runnable_t){.function = longbasictask,
            .arg = &tab[i],
            .argsz = sizeof(int)})== 0);

    raise(SIGINT);
    return 0;
}

static char* test_sigint6()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 4) == 0);
    int tab[testsigint5int];
    for(int i = 0; i < testsigint5int; i++) tab[i] = i;

    for(int i = 0; i < testsigint5int; i++)
        assert(defer(&pool, (runnable_t){.function = basictask,
                .arg = &tab[i],
                .argsz = sizeof(int)})== 0);

    raise(SIGINT);
    return 0;
}

static char* test_sigint7()
{
    thread_pool_t pool;
    assert(thread_pool_init(&pool, 5000) == 0);
    int tab[testsigint5int*1000];
    for(int i = 0; i < testsigint5int*1000; i++) tab[i] = i;

    for(int i = 0; i < testsigint5int*1000; i++)
        assert(defer(&pool, (runnable_t){.function = basictaskeps,
                .arg = &tab[i],
                .argsz = sizeof(int)})== 0);

    raise(SIGINT);
    return 0;
}



static char* test_wielepul1()
{
    int pol_no = 10;
    int tasks = 10;
    thread_pool_t* pools = malloc(sizeof(thread_pool_t) * pol_no);
    for(int j = 0; j < pol_no; j++)
        assert(thread_pool_init(&pools[j],4) == 0);

    future_t* futures = malloc(sizeof(future_t) * tasks);
    int err;
    int k = 0;
    err = async(&pools[0], &futures[0],
                (callable_t){.function = add_longmickiewiczwielkimpoetabyl, .arg = &k, .argsz = sizeof(int)});
    assert(err == 0);
    for(int j = 1; j < tasks; j++)
    {
        assert(map(&pools[j%pol_no], &futures[j], &futures[j-1], add_longmickiewiczwielkimpoetabyl) == 0);
    }
    int* val = await(&futures[tasks-1]);
    assert(*val == pol_no*44);
    free(val);

    for(int j = 0; j < tasks-1; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }

    for(int j = 0; j < pol_no; j++)
        thread_pool_destroy(&pools[j]);

    free(pools);
    free(futures);
    return 0;
}


static char* test_wielepul2()
{
    int pol_no = 10;
    int tasks = 10;
    thread_pool_t* pools = malloc(sizeof(thread_pool_t) * pol_no);
    for(int j = 0; j < pol_no; j++)
        assert(thread_pool_init(&pools[j],4) == 0);

    future_t* futures = malloc(sizeof(future_t) * tasks);
    int err;
    int k = 0;
    err = async(&pools[0], &futures[0],
                (callable_t){.function = add_longmickiewiczwielkimpoetabyl, .arg = &k, .argsz = sizeof(int)});
    assert(err == 0);
    for(int j = 1; j < tasks; j++)
    {
        assert(map(&pools[j%pol_no], &futures[j], &futures[j-1], add_longmickiewiczwielkimpoetabyl) == 0);
    }
    int* val = await(&futures[tasks-1]);
    assert(*val == tasks*44);
    free(val);

    for(int j = 0; j < tasks-1; j++)
    {
        int* v = await(&futures[j]);
        free(v);
    }

    for(int j = 0; j < pol_no; j++)
        thread_pool_destroy(&pools[j]);

    free(pools);
    free(futures);
    return 0;
}


static char* test_wielepul3() {

    thread_pool_t p1;
    thread_pool_t p2;
    future_t f1;
    future_t f2;
    assert(thread_pool_init(&p1,1) == 0);
    assert(thread_pool_init(&p2,1) == 0);
    int N = 34;
    int err;
    err = async(&p1, &f1,
                (callable_t){.function = add_longmickiewiczwielkimpoetabyl, .arg = &N, .argsz = sizeof(int)});
    assert(map(&p2, &f2, &f1, verylongadd10) == 0);
    thread_pool_destroy(&p2);
    thread_pool_destroy(&p1);
    int* val = await(&f2);
    assert(*val==aimiejegotoczterdziesciicztery * 2);
    free(val);
    int* val2 = await(&f1);
    free(val2);
    assert(err==0);

    return 0;
}

static char *test_antonimap() {
    thread_pool_t pool;
    future_t future;
    assert(!thread_pool_init(&pool, 1));

    int N = 2;
    //printf("&n: %p\n", &n);
    future_t future1, future2, future3;
    assert(!async(&pool, &future,
                  (callable_t) {.function = squared, .arg = &N, .argsz = sizeof(int)}));
    assert(!map(&pool, &future1, &future, squared));
    assert(!map(&pool, &future2, &future1, squared));
    assert(!map(&pool, &future3, &future2, squared));
    int *m = await(&future3);

    mu_assert("expected 256 * 256", *m == 256 * 256);
    free(m);
    int* v1 = await(&future1);
    int* v2 = await(&future2);
    int* v = await(&future);
    free(v1);
    free(v2);
    free(v);
    thread_pool_destroy(&pool);
    return 0;
}


static void *squared_free(void *arg, size_t argsz __attribute__((unused)),
                          size_t *retsz __attribute__((unused))) {
    int n = *(int *) arg;
    int *ret = malloc(sizeof(int));
    *ret = n * n;
    free(arg);
    return ret;
}

static void *div2_free(void *arg, size_t argsz __attribute__((unused)),
                       size_t *retsz __attribute__((unused))) {
    int n = *(int *) arg;
    int *ret = malloc(sizeof(int));
    *ret = n / 2;
    free(arg);
    return ret;
}


static char *test_map_simple() {
    thread_pool_t pool;
    thread_pool_init(&pool, 2);

    future_t tab[5];

    int n = 2;
    async(&pool, &tab[0],
          (callable_t) {.function = squared, .arg = &n, .argsz = sizeof(int)});
    map(&pool, &tab[1], &tab[0], squared_free);
    map(&pool, &tab[2], &tab[1], div2_free);
    map(&pool, &tab[3], &tab[2], squared_free);
    map(&pool, &tab[4], &tab[3], squared_free);
    int *m = await(&tab[4]);

    mu_assert("expected 64 * 64", *m == 64 * 64);
    free(m);

    thread_pool_destroy(&pool);
    return 0;
}

static char *test_map_simple2() {
    thread_pool_t pool;
    thread_pool_init(&pool, 1);

    future_t tab[5];

    int n = 2;
    async(&pool, &tab[0],
          (callable_t) {.function = squared, .arg = &n, .argsz = sizeof(int)});
    map(&pool, &tab[1], &tab[0], squared_free);
    map(&pool, &tab[2], &tab[1], div2_free);
    map(&pool, &tab[3], &tab[2], squared_free);
    int k = 0;
    for (int i = 0; i < 12341241; i++) {
        k = k * 12 + 1312;
        k %= 31209312;
    }
    map(&pool, &tab[4], &tab[3], squared_free);
    int *m = await(&tab[4]);

    mu_assert("nie", k != *m);
    mu_assert("expected 64 * 64", *m == 64 * 64);
    free(m);

    thread_pool_destroy(&pool);
    return 0;
}


static char *test_map_simple3() {
    thread_pool_t pool;
    thread_pool_init(&pool, 1);

    future_t tab[5];

    int n = 2;
    async(&pool, &tab[0],
          (callable_t) {.function = squared, .arg = &n, .argsz = sizeof(int)});
    map(&pool, &tab[1], &tab[0], squared_free);
    map(&pool, &tab[2], &tab[1], div2_free);
    map(&pool, &tab[3], &tab[2], squared_free);
    map(&pool, &tab[4], &tab[3], squared_free);
    int *mm = await(&tab[4]);
    mu_assert("expected 64 * 64", *mm == 64 * 64);
    free(mm);

    future_t tab2[5];
    int n2 = 3;
    async(&pool, &tab2[0],
          (callable_t) {.function = squared, .arg = &n2, .argsz = sizeof(int)});
    map(&pool, &tab2[1], &tab2[0], squared_free);
    map(&pool, &tab2[2], &tab2[1], div2_free);
    map(&pool, &tab2[3], &tab2[2], squared_free);
    map(&pool, &tab2[4], &tab2[3], squared_free);
    int *m = await(&tab2[4]);

    mu_assert("expected 2560000", *m == 2560000);
    free(m);

    thread_pool_destroy(&pool);
    return 0;
}

static char* test_sigint8() {
    int err = 0;
    for(int k = 0; k < 1000; ++k) {
        struct timespec ts = {
                0,
                2000000
        };
        thread_pool_t pool;
        thread_pool_init(&pool, 1000);
        nanosleep(&ts, NULL);
        raise(SIGINT);
        //if(k % 100 == 0) printf("%d\n", k);
    }
    return 0;
}

static char* test_sigint9()
{
    {
        thread_pool_t pool;
        assert(thread_pool_init(&pool, 100) == 0);
        sleep(1);
        raise(SIGINT);
//        thread_pool_destroy(&pool);
    }

  /*  {//ten test powinien crashnąć SIGINTEM
        thread_pool_t pool;
        assert(thread_pool_init(&pool, 100) == 0);
        sleep(1);
        thread_pool_destroy(&pool);
        raise(SIGINT);
    }*/


    {
        thread_pool_t pool;
        assert(thread_pool_init(&pool, 100) == 0);
        raise(SIGINT);
//        thread_pool_destroy(&pool);
    }

   /* {//ten test powinien crashnąć SIGINTEM
        thread_pool_t pool;
        assert(thread_pool_init(&pool, 100) == 0);
        thread_pool_destroy(&pool);
        raise(SIGINT);
    } */
    return 0;
}

static char* test_sigint10() {
    int err = 0;
    for(int k = 0; k < 1e6; ++k) {
        struct timespec ts = {
                0,
                10
        };
        thread_pool_t pool;
        thread_pool_init(&pool, 1);
        nanosleep(&ts, NULL);
        raise(SIGINT);
        if(k % 100 == 0) printf("%d\n", k);
    }
    return 0;
}

static char *all_tests() {

    for(size_t j = 0; j < 2; j++)
    {
        printf("Test upcoming %zu \n", j);
        mu_run_test(test_wielepul1); printf("passed\n");
        mu_run_test(test_wielepul2); printf("passed\n");
        mu_run_test(test_wielepul3); printf("passed\n");
        mu_run_test(test_await_simple); printf("passed\n");
        mu_run_test(test_wielokrotne); printf("passed\n");
        mu_run_test(test_dlugie); printf("passed\n");
        mu_run_test(test_antonimap); printf("passed\n");
        mu_run_test(test_map_simple); printf("passed\n");
        mu_run_test(test_map_simple2); printf("passed\n");
        mu_run_test(test_map_simple3); printf("passed\n");
        mu_run_test(test_map_basic); printf("passed\n");
        mu_run_test(test_map_waiting); printf("passed\n");
        mu_run_test(test_multimap); printf("passed\n");
        mu_run_test(test_multimapmieszany);printf("passed\n");
        mu_run_test(test_multimapszybki);printf("passed\n");
        mu_run_test(test_multimapmieszanyszybki); printf("passed\n");
        mu_run_test(test_sigint1); printf("passed (sigint)\n");
        mu_run_test(test_sigint2); printf("passed (sigint)\n");
        mu_run_test(test_sigint3); printf("passed (sigint)\n");
        mu_run_test(test_sigint4); printf("passed (sigint)\n");
        mu_run_test(test_sigint5); printf("passed (sigint)\n");
        mu_run_test(test_sigint6); printf("passed (sigint)\n");
        mu_run_test(test_sigint7); printf("passed (sigint)\n");
        mu_run_test(test_sigint8); printf("passed (sigint)\n");
        mu_run_test(test_sigint9); printf("passed (sigint)\n");
//        mu_run_test(test_sigint10);
    }


    return 0;
}


int main() {

  char *result = all_tests();
  if (result != 0) {
    printf(__FILE__ " %s\n", result);
  } else {
    printf(__FILE__ " ALL TESTS PASSED\n");
  }
  printf(__FILE__ " Tests run: %d\n", tests_run);

  return result != 0;
}
