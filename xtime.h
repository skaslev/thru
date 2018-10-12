#ifndef XTIME_H_
#define XTIME_H_

#include <time.h>

static inline void timespec_diff(
	const struct timespec *t0, const struct timespec *t1,
	struct timespec *dt)
{
	if (t1->tv_nsec - t0->tv_nsec < 0) {
		dt->tv_sec = t1->tv_sec - t0->tv_sec - 1;
		dt->tv_nsec = t1->tv_nsec - t0->tv_nsec + 1000000000;
	} else {
		dt->tv_sec = t1->tv_sec - t0->tv_sec;
		dt->tv_nsec = t1->tv_nsec - t0->tv_nsec;
	}
}

static inline double timespec_usec(const struct timespec *t)
{
	return 1e6 * t->tv_sec + 1e-3 * t->tv_nsec;
}

static inline double timespec_sec(const struct timespec *t)
{
	return t->tv_sec + 1e-9 * t->tv_nsec;
}

#endif // XTIME_H_
