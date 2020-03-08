#include "threadtimestat.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Функция получения времени запуска, времени выполнения потока,
// при указанном количестве операций
ThreadStat threadTimeStat(pthread_func thread_func, size_t op_count) {
	pthread_t thread;
	ThreadArg thread_arg = (ThreadArg){op_count, 0.0};
	int err = 0;

	// Расчёт времени запуска потока
	struct timespec launch_start, launch_stop;
	clock_gettime(CLOCK_MONOTONIC, &launch_start);

	err = pthread_create(&thread, NULL, thread_func, (void*)&thread_arg);
	if (err != 0)
        throwErr("Error: thread create error!");

	clock_gettime(CLOCK_MONOTONIC, &launch_stop);

	pthread_join(thread, NULL);
	if (err != 0)
        throwErr("Error: thread join error!");

	return (ThreadStat){clocktimeDifference(launch_start, launch_stop), 
						thread_arg.elapsed_time};
}