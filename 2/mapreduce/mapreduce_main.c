#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mapreduce.h"

#define DEMO_ARGS_COUNT 3
#define TEST_ARGS_COUNT 5

#define RESULT_FILENAME "result.txt"

#define BILLION 1.0E+9

#define clocktimeDifference(start, stop)            \
	1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

static double mapFuncInc(double val) {
	return val + 1;
}

static double reduceFuncPlus(double result, double b) {
	return result + b;
}

static double reduceFuncMult(double result, double b) {
	return result * b;
}

static double* arrayCreate(uint32_t size) {
	double* A = (double*)malloc(sizeof(double) * size);
	return A;
}

static void arrayRandInit(double* A, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		A[i] = rand() % size;
}

static void arrayCopy(double* dest, double* src, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		dest[i] = src[i];
}

static void arryPrint(double* A, uint32_t size) {
	for (uint32_t i = 0; i != size; ++i)
		printf("%g ", A[i]);
	printf("\n");
}

static void testResultOutput(FILE* fp, uint8_t threads_count, 
                             uint32_t array_size_min, uint32_t array_size_max, 
							 size_t measure_count) {
	fprintf(fp, "size\tthreads: 1\tthreads: 2\tthreads: 3\tthreads: 4\n");

	for (uint32_t size = array_size_min; size < array_size_max; size *= 10) {
		double* A_src = arrayCreate(size);
		double* A = arrayCreate(size);

		arrayRandInit(A_src, size);

		fprintf(fp, "%d:\t", size);

		for (uint8_t i = 0; i != threads_count; ++i) {
			arrayCopy(A, A_src, size);

			double elapsed_time = 0;
			for (size_t j = 0; j != measure_count; ++j) {
				struct timespec start, stop;
				clock_gettime(CLOCK_MONOTONIC, &start);

				mapReduceArray(A, size, mapFuncInc, reduceFuncPlus, i + 1);

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

static void demonstration(int argc, char* argv[]) {
	if (argc < DEMO_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size>\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

    uint8_t threads_count = atoi(argv[1]);
	uint32_t size = atoi(argv[2]);

	double* A = arrayCreate(size);
	arrayRandInit(A, size);

	printf("Source array:\n");
	arryPrint(A, size);

	struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

	double result = mapReduceArray(A, size, mapFuncInc, 
	                               reduceFuncMult, threads_count);

	clock_gettime(CLOCK_MONOTONIC, &stop);
	double elapsed_time = clocktimeDifference(start, stop);

	printf("\nMap result:\n");
	arryPrint(A, size);

	printf("\nMapReduce result:\n%g\n", result);

	printf("\nElapsed time: %lf\n", elapsed_time);

	free(A);
}

static void test(int argc, char* argv[]) {
	if (argc < TEST_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size min>" 
							   "<array size max> <measure count>\n");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));

	uint8_t threads_count = atoi(argv[1]);
	uint32_t array_size_min = atoi(argv[2]);
	uint32_t array_size_max = atoi(argv[3]);
	size_t measure_count = atoi(argv[4]);

	FILE* fp = fopen(RESULT_FILENAME, "w");

	printf("Program execution...\n");
	testResultOutput(fp, threads_count, array_size_min, 
	                 array_size_max, measure_count);
	printf("Done.\n");

	fclose(fp);
}

int main(int argc, char* argv[]) {
	#ifdef TEST 
	test(argc, argv);
	#else 
	demonstration(argc, argv);
	#endif

    return 0;
}