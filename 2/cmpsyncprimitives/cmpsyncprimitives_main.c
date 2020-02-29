#include <stdio.h>
#include <stdint.h>
#include <float.h>

#include "cmpsyncprimitives.h"

#define ARGS_COUNT 4 

#define TABLE_FILENAME "result.txt"

typedef enum ResultType {
	RT_AVG, 
	RT_MIN
} ResultType;

static void tableOutput(FILE* fp, ResultType result_type, size_t threads_count, size_t measure_count) {
	double time_init = 0;
	switch (result_type) {
		case RT_AVG: time_init = 0;       break;
		case RT_MIN: time_init = DBL_MAX; break;
		default: result_type = RT_AVG;
	}

	fprintf(fp, "thrds:\tmutex:\tspin:\n");

	for (size_t i = 0; i != threads_count; ++i) {
		double mutex_elapsed_time = time_init;
		double spin_elapsed_time = time_init;

		for (size_t j = 0; j != measure_count; ++j) {
			double mutex_tmp = primitiveTimeStat(MUTEX, i + 1);
			double spin_tmp = primitiveTimeStat(SPINLOCK, i + 1);

			switch (result_type) {
				case RT_AVG: {
					mutex_elapsed_time += mutex_tmp;
					spin_elapsed_time += spin_tmp;
					break;
				}
				case RT_MIN: {
					if (mutex_tmp < mutex_elapsed_time)
						mutex_elapsed_time = mutex_tmp;
					if (spin_tmp < spin_elapsed_time)
						spin_elapsed_time = spin_tmp;
					break;
				}
			}
		}

		if (result_type == RT_AVG) {
			mutex_elapsed_time /= measure_count;
			spin_elapsed_time /= measure_count;
		}

		fprintf(fp, "%zu\t%lf\t%lf\n", i + 1, mutex_elapsed_time, spin_elapsed_time);
	}
}

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <measure count> <result type>\n");
		fprintf(stderr, "(Result type: 0 - avg; 1 - min)\n");
		exit(EXIT_FAILURE);
	}

    uint8_t threads_count = atoi(argv[1]);
	size_t measure_count = atol(argv[2]);
	uint8_t result_type = atoi(argv[3]);
	
	FILE* fp = fopen(TABLE_FILENAME, "w");
	tableOutput(fp, result_type, threads_count, measure_count);
	fclose(fp);

    return 0;
}