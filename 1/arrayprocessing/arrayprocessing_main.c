#include "arrayprocessing.h"

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <time.h>

#define ARGS_COUNT 6

#define RESULT_FILENAME "result.txt"

#define BILLION 1.0E+9

#define clocktimeDifference(start, stop) \
	1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

static double sqrFunc(double val) {
	return val * val;
}

static double expFunc(double val) {
	return exp(val);
}

static double revFunc(double val) {
	return 1 / val;
}

static void resultOutput(FILE* fp, uint8_t threads_count, uint32_t array_size_min, uint32_t array_size_max, uint8_t func_num, size_t measure_count) {
	func_ptr array_func = sqrFunc;
	switch (func_num) {
		case 0: array_func = sqrFunc; break;
		case 1: array_func = expFunc; break;
		case 2: array_func = revFunc; break;
	}

	fprintf(fp, "size\tthreads: 1\tthreads: 2\tthreads: 3\tthreads: 4\n");

	
	for (uint32_t array_size = array_size_min; array_size < array_size_max; array_size *= 10) {
		double* A_src = arrayCreate(array_size);
		double* A = arrayCreate(array_size);

		arrayInit(A_src, array_size);

		fprintf(fp, "%d:\t", array_size);

		for (uint8_t i = 0; i != threads_count; ++i) {
			arrayCopy(A, A_src, array_size);

			double elapsed_time = DBL_MAX;
			for (size_t j = 0; j != measure_count; ++j) {
				struct timespec start, stop;
				clock_gettime(CLOCK_REALTIME, &start);

				arrayProcessing(A, array_size, i + 1, array_func);

				clock_gettime(CLOCK_REALTIME, &stop);

				double tmp_time = clocktimeDifference(start, stop);
				if (tmp_time < elapsed_time)
					elapsed_time = tmp_time;
			}

			fprintf(fp, "%lf\t", elapsed_time);
		}

		fprintf(fp, "\n");

		free(A_src);
		free(A);
	}
}

int main(int argc, char* argv[]) {
	if (argc != ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size min> <array size max> <func number> <measure count>\n");
		fprintf(stderr, "(Func number: 0 - sqr, 1 - cube, 2 - revers)\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	uint8_t threads_count = atoi(argv[1]);
	uint32_t array_size_min = atoi(argv[2]);
	uint32_t array_size_max = atoi(argv[3]);
	uint8_t func_num = atoi(argv[4]);
	size_t measure_count = atoi(argv[5]);

	FILE* fp = fopen(RESULT_FILENAME, "w");

	printf("Program execution...\n");
	resultOutput(fp, threads_count, array_size_min, array_size_max, func_num, measure_count);
	printf("Done.\n");

	fclose(fp);

	return 0;
}