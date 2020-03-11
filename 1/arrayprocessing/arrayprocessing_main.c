#include "arrayprocessing.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

#define ARGS_COUNT 6

#define RESULT_FILENAME "result.txt"

#define BILLION 1.0E+9

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop) 			\
	1.0 * (stop.tv_sec - start.tv_sec) + 			\
	1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

// Функции обработки элементов массива
static double sqrFunc(double val) {
	return val * val;
}

static double expFunc(double val) {
	return exp(val);
}

static double revFunc(double val) {
	return 1 / val;
}

// Функция расчёта времени обработки элементов массива на указанном количестве потоков,
// с указанным начальным и конечным размером массива
static void resultOutput(FILE* fp, uint8_t threads_count, 
						 size_t array_size_min, size_t array_size_max, 
						 uint8_t func_num, size_t measure_count) {
	// Определение функции обработки элемента массива
	func_ptr array_func = sqrFunc;
	switch (func_num) {
		case 0: array_func = sqrFunc; break;
		case 1: array_func = expFunc; break;
		case 2: array_func = revFunc; break;
	}

	// Составление таблицы
	fprintf(fp, "size:\tthreads: 1\tthreads: 2\tthreads: 3\tthreads: 4\n");

	// Увеличение размера массива
	for (size_t array_size = array_size_min; array_size < array_size_max; array_size *= 10) {
		// Создание исходного массива и массива для многократной обработки,
		// для обеспечения одинаковых данных на всех испытаниях
		double* A_src = arrayCreate(array_size);
		double* A = arrayCreate(array_size);

		// Инициализация исходного массива
		arrayRandInit(A_src, array_size);

		fprintf(fp, "%zu\t", array_size);

		for (uint8_t i = 0; i != threads_count; ++i) {
			// Копирование данных из исходного массива в массив обработки
			arrayCopy(A, A_src, array_size);

			// Повторяем замеры указанное число раз
			double elapsed_time = 0;
			for (size_t j = 0; j != measure_count; ++j) {
				struct timespec start, stop;
				clock_gettime(CLOCK_MONOTONIC, &start);

				arrayProcessing(A, array_size, array_func, i + 1);

				clock_gettime(CLOCK_MONOTONIC, &stop);

				elapsed_time += clocktimeDifference(start, stop);
			}

			fprintf(fp, "%lf\t", elapsed_time / measure_count);
		}

		fprintf(fp, "\n");

		free(A_src);
		free(A);
	}
}

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size min> "
						"<array size max> <func number> <measure count>\n");
		fprintf(stderr, "(Func number: 0 - sqr, 1 - cube, 2 - revers)\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	uint8_t threads_count = atoi(argv[1]);
	size_t array_size_min = atoi(argv[2]);
	size_t array_size_max = atoi(argv[3]);
	uint8_t func_num = atoi(argv[4]);
	size_t measure_count = atoi(argv[5]);

	FILE* fp = fopen(RESULT_FILENAME, "w");

	printf("Program execution...\n");
	resultOutput(fp, threads_count, array_size_min, 
				 array_size_max, func_num, measure_count);
	printf("Done.\n");

	fclose(fp);

	return 0;
}