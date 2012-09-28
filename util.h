#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

/* Equivalent to malloc, but exit's on failure */
void *salloc(size_t bytes);

/* Equivalent to realloc, but exit's on failure */
void *srealloc(void *mem, size_t bytes);

#endif
