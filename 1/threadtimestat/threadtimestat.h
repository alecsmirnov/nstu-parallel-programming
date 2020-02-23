#ifndef THREADTIMESTAT_H
#define THREADTIMESTAT_H

#include <time.h>
#include <pthread.h>

#define FUNCTION_CALL_TIME 8.0E-10
#define BILLION            1.0E+9

#define clocktimeDifference(start, stop) \
	1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

typedef double (*func_t)(double a, double b);

typedef struct ThreadStat {
	double launch_time;
	double elapsed_time;
} ThreadStat;

typedef struct ThreadArg {
	size_t op_count;

	double elapsed_time;
} ThreadArg;

static void* threadFunc(void* arg) {
	ThreadArg* thread_arg = (ThreadArg*)arg;

	double a = 17;
	double b = 5.125013;

	struct timespec start, stop; 
	clock_gettime(CLOCK_REALTIME, &start);

	for (size_t i = 0; i != thread_arg->op_count; ++i)
		a *= b;

	clock_gettime(CLOCK_REALTIME, &stop);
	thread_arg->elapsed_time = clocktimeDifference(start, stop);

	if (0 < thread_arg->op_count)
		thread_arg->elapsed_time -= FUNCTION_CALL_TIME * thread_arg->op_count;

	pthread_exit(NULL);
}

ThreadStat threadTimeStat(size_t op_count);

#endif