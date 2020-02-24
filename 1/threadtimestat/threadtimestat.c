#include "threadtimestat.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

// Код прохождения обработки
#define PTHREAD_PASS 0

// Функция обработки ошибок
#define pthreadErrorHandle(test_code, pass_code, err_msg) do {		\
	if (test_code != pass_code) {									\
		fprintf(stderr, "%s: %s\n", err_msg, strerror(test_code));	\
		exit(EXIT_FAILURE);											\
	}																\
} while (0)

// Функция получения времени запуска, времени выполнения потока,
// при указанном количестве операций
ThreadStat threadTimeStat(size_t op_count) {
	pthread_t thread_id;
	ThreadArg thread_arg = (ThreadArg){op_count, 0.0};

	// Расчёт времени запуска потока
	struct timespec launch_start, launch_stop;
	clock_gettime(CLOCK_REALTIME, &launch_start);

	int err = pthread_create(&thread_id, NULL, threadFunc, (void*)&thread_arg);
	pthreadErrorHandle(err, PTHREAD_PASS, "Thread create error");

	clock_gettime(CLOCK_REALTIME, &launch_stop);

	pthread_join(thread_id, NULL);
	pthreadErrorHandle(err, PTHREAD_PASS, "Thread join error");

	return (ThreadStat){clocktimeDifference(launch_start, launch_stop), thread_arg.elapsed_time};
}
