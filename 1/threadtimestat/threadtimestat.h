#ifndef THREADTIMESTAT_H
#define THREADTIMESTAT_H

#include <time.h>

#define FUNCTION_CALL_TIME 8.0E-10
#define BILLION            1.0E+9

typedef double (*func_t)(double a, double b);

typedef struct ThreadStat {
	double launch_time;
	double elapsed_time;
} ThreadStat;

static inline double clocktimeDifference(struct timespec start, struct timespec stop) {
	return 1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION;
}

ThreadStat threadTimeStat(func_t op_func, size_t op_count, double a, double b);


#endif
