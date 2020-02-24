#ifndef THREADTIMESTAT_H
#define THREADTIMESTAT_H

#include <time.h>
#include <pthread.h>

#define BILLION 1.0E+9

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop) \
	1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Статистика потока
typedef struct ThreadStat {
	double launch_time;		// Время запуска
	double elapsed_time;	// Время выполнения
} ThreadStat;

// Параметры потока
typedef struct ThreadArg {
	// Входной аргумент
	size_t op_count;		// Количество операций

	// Выходной аргумент
	double elapsed_time;	// Время выполнения
} ThreadArg;

static void* threadFunc(void* arg) {
	ThreadArg* thread_arg = (ThreadArg*)arg;

	double a = 17;
	double b = 5.125013;

	// Расчёт времени обработки указанного кол-ва операций умножения
	struct timespec start, stop; 
	clock_gettime(CLOCK_REALTIME, &start);

	for (size_t i = 0; i != thread_arg->op_count; ++i)
		a *= b;

	clock_gettime(CLOCK_REALTIME, &stop);
	thread_arg->elapsed_time = clocktimeDifference(start, stop);

	pthread_exit(NULL);
}

// Функция получения времени запуска, времени выполнения потока,
// при указанном количестве операций
ThreadStat threadTimeStat(size_t op_count);

#endif
