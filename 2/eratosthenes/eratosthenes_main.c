#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "eratosthenes.h"

#define ARGS_COUNT 4 

#define BILLION 1.0E+9

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop) \
	1.0 * (stop.tv_sec - start.tv_sec) + 1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <threads count> <n> <chunk size>\n");
		exit(EXIT_FAILURE);
	}

    uint8_t threads_count = atoi(argv[1]);
	size_t n = atol(argv[2]);
	size_t chunk_size = atol(argv[3]);

    Prime prime;
    primeDataInit(&prime, threads_count, n, chunk_size);

    struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

    sieveStart(&prime, threads_count, n, chunk_size);

    clock_gettime(CLOCK_MONOTONIC, &stop);
    double elapsed_time = clocktimeDifference(start, stop);

    for (size_t i = 0; i != prime.n; ++i)
        if (prime.data[i])
            printf("%zu ", prime.data[i]);
    printf("\n");
    
    printf("\nElapsed time: %lf\n", elapsed_time);

    primeDataFree(&prime);

    return 0;
}