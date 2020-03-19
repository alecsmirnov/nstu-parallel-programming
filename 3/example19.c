#include <stdio.h>
#include <time.h>
#include <omp.h>

#define OP_COUNT 100000000
#define A_SIZE   100

#define BILLION 1.0E+9

// Функция вычета разности между временными величинами
#define clocktimeDifference(start, stop)            \
    1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

int f(int val) {
    double result = 0;
    for (size_t i = 0; i < OP_COUNT; ++i)
        result += 1.0 / (i + 1);

    return (int)(val * result);
}

int main(int argc, char* argv[]) {
    int a[A_SIZE], b[A_SIZE];

    // Инициализация массива b
    for(int i = 0; i < A_SIZE; i++)
        b[i] = i;
    
    struct timespec start, stop;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Директива OpenMP для распараллеливания цикла
    #pragma omp parallel for
    for(int i = 0; i < A_SIZE; i++) {
        a[i] = f(b[i]);
        b[i] = 2 * a[i];
    }

    int result = 0;

    // Далее значения a[i] и b[i] используются, например, так:
    #pragma omp parallel for reduction(+ : result)
    for(int i = 0; i < A_SIZE; i++)
        result += (a[i] + b[i]);

    clock_gettime(CLOCK_MONOTONIC, &stop);

    printf("Result = %d\n", result);
    
    printf("\nElapsed time: %lf\n", clocktimeDifference(start, stop));

    return 0;
}
