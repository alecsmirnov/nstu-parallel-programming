#ifndef THREADTIMESTAT_H
#define THREADTIMESTAT_H

#include <time.h>
#include <pthread.h>

#define BILLION 1.0E+9

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop) \
	1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Указатель на потоковую функцию
typedef void* (*pthread_func)(void*);

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

// Функция получения времени запуска, времени выполнения потока,
// при указанном количестве операций
ThreadStat threadTimeStat(pthread_func thread_func, size_t op_count);

#endif
