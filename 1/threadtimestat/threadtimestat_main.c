#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>

#include "errorhandle.h"
#include "threadtimestat.h"

#define ARGS_COUNT 4

#define RESULT_FILENAME "result.txt"

static double multOpFunc(double a, double b) {
	return a * b;
}

static void resultOutput(FILE* fp, size_t measure_count, size_t op_start, size_t op_step, func_t op_func, double a, double b) {
	fprintf(fp, "op count:\tlaunch time:\telapsed time:\n");

	size_t op_count = op_start;
	ThreadStat min_time = (ThreadStat){DBL_MAX, DBL_MAX};
	while (min_time.elapsed_time <= min_time.launch_time) {
		min_time = (ThreadStat){DBL_MAX, DBL_MAX};

		for (size_t j = 0; j != measure_count; ++j) {
			ThreadStat thread_stat = threadTimeStat(op_func, op_count, a, b);

			if (thread_stat.launch_time < min_time.launch_time)
				min_time.launch_time = thread_stat.launch_time;

			if (thread_stat.elapsed_time < min_time.elapsed_time)
				min_time.elapsed_time = thread_stat.elapsed_time;
		}

		fprintf(fp, "%zu:\t", op_count);
		fprintf(fp, "%.14lf\t", min_time.launch_time);
		fprintf(fp, "%.14lf\n", min_time.elapsed_time);

		op_count += op_step;
	}
}

int main(int argc, char *argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of aguments!\n");
		fprintf(stderr, "Enter: <measure count> <start operation count> <step operation count>\n");
		exit(EXIT_FAILURE);
	}

	size_t measure_count = atoi(argv[1]);
	size_t op_start      = atoi(argv[2]);
	size_t op_step       = atoi(argv[3]);

	double a = 17;
	double b = 3.13;

	FILE* fp = fopen(RESULT_FILENAME, "w");

	printf("Program execution...\n");
	resultOutput(fp, measure_count, op_start, op_step, multOpFunc, a, b);
	printf("Done.\n");

	fclose(fp);

	return 0;
}
