#include <stdlib.h>
#include <unistd.h>

#include "eratosthenes.h"

#define ARGS_COUNT 4 

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
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

    return 0;
}