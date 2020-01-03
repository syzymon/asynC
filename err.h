#ifndef _ERR_
#define _ERR_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* print system call error message and terminate */
extern void syserr(int bl, const char *fmt, ...);

#endif
