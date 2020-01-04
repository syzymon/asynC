// http: // www.jera.com/techinfo/jtns/jtn002.html

#ifndef MINUNIT_H
#define MINUNIT_H

//#define JAKAS_DUZA_LICZBA 213769
#define JAKAS_DUZA_LICZBA 2137

#define mu_assert(message, test)                                               \
  do {                                                                         \
    if (!(test))                                                               \
      return message;                                                          \
  } while (0)

#define mu_run_test(test)                                                      \
  do {                                                                         \
    char *message = test();                                                    \
    tests_run++;                                                               \
    if (message)                                                               \
      return message;                                                          \
  } while (tests_run < JAKAS_DUZA_LICZBA)

extern int tests_run;

#endif