#include <stdio.h>
#include <math.h>

#include "jacobi.h"

#define ARGS_COUNT 3

static double** matrixCreate(size_t n) {
    double** a = (double**)malloc(sizeof(double*) * n);
    for (size_t i = 0; i != n; ++i)
        a[i] = (double*)malloc(sizeof(double) * n);

    return a;
}

static void matrixRandInit(double** a, size_t n) {
    for (size_t i = 0; i != n; ++i) {
        for (size_t j = 0; j != n; ++j)
            if (i != j)
                a[i][j] = 0.1 / (i + j);

        a[i][i] = 1;
    }
}

static double* arrayCreate(size_t n) {
    double* b = (double*)malloc(sizeof(double) * n);
    return b;
}

static void arrayRandInit(double* b, size_t n) {
    for (size_t i = 0; i != n; ++i)
        b[i] = sin(1);
}

int main(int argc, char* argv[]) {
    if (argc < ARGS_COUNT) {
        fprintf(stderr, "Wrong number of arguments!\n");
        fprintf(stderr, "Enter: <threads count> <n>\n");
        exit(EXIT_FAILURE);
    }

    uint8_t threads_count = atoi(argv[1]);
    size_t n = atol(argv[2]);

    double** a = matrixCreate(n);
    double* b = arrayCreate(n);

    matrixRandInit(a, n);
    arrayRandInit(b, n);

    double start = getWTime();

    double* x = jacobi(a, b, n, threads_count); 

    double stop = getWTime();

    printf("Elapsed time: %lf\n", stop - start);

    for (size_t i = 0; i != n; ++i)
        free(a[i]);
    free(a);
    free(b);
    free(x);

    return 0;
}