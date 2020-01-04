#include "err.h"

extern void syserr(int bl, const char *fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr, " (%d; %s)\n", bl, strerror(bl));
    exit(1);
}