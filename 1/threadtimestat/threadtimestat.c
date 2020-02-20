#include "threadtimestat.h"

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "errorhandle.h"

typedef struct ThreadArg {
	double a;
	double b;

	func_t op_func;
	size_t op_count;

	double elapsed_time;
} ThreadArg;

static void* threadFunc(void* arg) {
	ThreadArg* thread_arg = (ThreadArg*)arg;

	struct timespec start, stop;
	clock_gettime(CLOCK_REALTIME, &start);

	double res = 0;
	for (size_t i = 0; i != thread_arg->op_count; ++i)
		res = thread_arg->op_func(thread_arg->a, thread_arg->b);

	clock_gettime(CLOCK_REALTIME, &stop);
	thread_arg->elapsed_time = clocktimeDifference(start, stop);

	if (0 < thread_arg->op_count)
		thread_arg->elapsed_time -= FUNCTION_CALL_TIME * thread_arg->op_count;

	pthread_exit(NULL);
}

ThreadStat threadTimeStat(func_t op_func, size_t op_count, double a, double b) {
	pthread_t thread_id;
	ThreadArg thread_arg = (ThreadArg){a, b, op_func, op_count, 0.0};

	struct timespec launch_start, launch_stop;
	clock_gettime(CLOCK_REALTIME, &launch_start);

	int err = pthread_create(&thread_id, NULL, threadFunc, (void*)&thread_arg);
	errorHandle(err, PASS_CODE, "Thread create error");

	clock_gettime(CLOCK_REALTIME, &launch_stop);

	pthread_join(thread_id, NULL);
	errorHandle(err, PASS_CODE, "Thread join error");

	return (ThreadStat){clocktimeDifference(launch_start, launch_stop), thread_arg.elapsed_time};
}
