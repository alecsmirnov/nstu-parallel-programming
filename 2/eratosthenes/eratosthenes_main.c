#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "eratosthenes.h"

#define DEMO_ARGS_COUNT 4
#define TEST_ARGS_COUNT 5

#define RESULT_FILENAME "result.txt"

static void testResultOutput(FILE* fp, uint8_t threads_count, 
                             uint32_t array_size_min, uint32_t array_size_max, 
							 size_t measure_count) {
	fprintf(fp, "size\tthreads: 1\tthreads: 2\tthreads: 3\tthreads: 4\n");

	for (uint32_t size = array_size_min; size < array_size_max; size *= 10) {
		fprintf(fp, "%d:\t", size);

		for (uint8_t i = 0; i != threads_count; ++i) {
			double elapsed_time = 0;
			
			for (size_t j = 0; j != measure_count; ++j) {
				PrimeNumbers prime_numbers = sieveStart(i + 1, size, size / 10);

				elapsed_time += prime_numbers.elapsed_time;
				free(prime_numbers.data);
			}

			fprintf(fp, "%lf\t", elapsed_time / measure_count);
		}

		fprintf(fp, "\n");
	}
}

static void test(int argc, char* argv[]) {
	if (argc < TEST_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <array size min> " 
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

static void demonstration(int argc, char* argv[]) {
	if (argc < DEMO_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <n limit> <chunk size>\n");
		exit(EXIT_FAILURE);
	}

    uint8_t threads_count = atoi(argv[1]);
	size_t n = atol(argv[2]);
	size_t chunk_size = atol(argv[3]);

    PrimeNumbers prime_numbers = sieveStart(threads_count, n, chunk_size);

    printPrimeNumbers(&prime_numbers);

    free(prime_numbers.data);
}

int main(int argc, char* argv[]) {
	#ifdef TEST 
	test(argc, argv);
	#else 
	demonstration(argc, argv);
	#endif

    return 0;
}