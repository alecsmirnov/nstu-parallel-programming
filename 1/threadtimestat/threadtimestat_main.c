#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>

#include "threadtimestat.h"

// Количество аргументов командной строки
#define ARGS_COUNT 4

// Выходной файл для результато тестирования
#define RESULT_FILENAME "result.txt"

static void* threadFunc(void* arg) {
	ThreadArg* thread_arg = (ThreadArg*)arg;

	double a = 17;
	double b = 5.125013;

	// Расчёт времени обработки указанного кол-ва операций умножения
	struct timespec start, stop; 
	clock_gettime(CLOCK_MONOTONIC, &start);

	for (size_t i = 0; i != thread_arg->op_count; ++i)
		a *= b;

	clock_gettime(CLOCK_MONOTONIC, &stop);
	thread_arg->elapsed_time = clocktimeDifference(start, stop);

	pthread_exit(NULL);
}

// Функция поиска оптимального количества операций для порождения потока,
// с выводом всех шагов поиска
static void resultOutput(FILE* fp, size_t op_start, size_t op_step, size_t measure_count) {
	fprintf(fp, "op count:\tlaunch time:\telapsed time:\n");

	size_t op_count = op_start;
	ThreadStat min_time = (ThreadStat){DBL_MAX, DBL_MAX};
	// Пока время выполнения меньше времени запуска
	while (min_time.elapsed_time <= min_time.launch_time) {
		min_time = (ThreadStat){DBL_MAX, DBL_MAX};

		// Повторяем замеры указанное число раз и находим минимальное значение
		// для каждой величины (исключаем посторонние процессы)
		for (size_t j = 0; j != measure_count; ++j) {
			ThreadStat thread_stat = threadTimeStat(threadFunc, op_count);

			if (thread_stat.launch_time < min_time.launch_time)
				min_time.launch_time = thread_stat.launch_time;

			if (thread_stat.elapsed_time < min_time.elapsed_time)
				min_time.elapsed_time = thread_stat.elapsed_time;
		}

		fprintf(fp, "%zu:\t", op_count);
		fprintf(fp, "%.14lf\t", min_time.launch_time);
		fprintf(fp, "%.14lf\n", min_time.elapsed_time);

		// Увеличиваем количество операций
		op_count += op_step;
	}
}

int main(int argc, char *argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of aguments!\n");
		fprintf(stderr, "Enter: <start operation count> <step operation count> <measure count>\n");
		exit(EXIT_FAILURE);
	}

	size_t op_start = atoi(argv[1]);
	size_t op_step = atoi(argv[2]);
	size_t measure_count = atoi(argv[3]);

	FILE* fp = fopen(RESULT_FILENAME, "w");

	printf("Program execution...\n");
	resultOutput(fp, op_start, op_step, measure_count);
	printf("Done.\n");

	fclose(fp);

	return 0;
}