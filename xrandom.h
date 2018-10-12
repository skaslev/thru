#ifndef XRANDOM_H_
#define XRANDOM_H_

#include <stdlib.h>

static inline void fill_random(void *buf, size_t len)
{
	size_t i, n = len / sizeof(int);
	int *p = buf;

	for (i = 0; i < n; i++)
		p[i] = rand();
}

#endif // XRANDOM_H_
