#ifndef ERATOSTHENES_H
#define ERATOSTHENES_H

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

typedef struct Work Work;

typedef struct Prime {
    uint8_t threads_count;
    size_t n;
    size_t chunk_size;

    size_t* data;

    pthread_t* threads;
    struct Work* works;
} Prime;

void primeDataInit(Prime* prime, uint8_t threads_count, size_t n, size_t chunk_size);

void sieveStart(Prime* prime, uint8_t threads_count, size_t n, size_t chunk_size);

void primeDataFree(Prime* prime);

#endif