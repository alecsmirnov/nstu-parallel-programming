#include <stdio.h>
#include <omp.h>

#define OP_COUNT 100000000
#define A_SIZE   100

int f(int val) {
    double result = 0;
    for (size_t i = 0; i < OP_COUNT; ++i)
        result += val / (i + 1);

    return (int)result;
}

int main(int argc, char* argv[]) {
    int a[A_SIZE], b[A_SIZE];

    // Инициализация массива b
    for(int i = 0; i < A_SIZE; i++)
        b[i] = i;
    
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

    printf("Result = %d\n", result);

    return 0;
}