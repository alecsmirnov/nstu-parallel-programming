#ifndef JACOBI_H
#define JACOBI_H

#include <stdlib.h>
#include <stdint.h>

#define ITERS_COUNT_MAX 100
#define EPS 0.001

double getWTime();

double* jacobi(double** a, double* b, size_t n, uint8_t threads_count);

#endif