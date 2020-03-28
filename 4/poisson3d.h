#ifndef POISSON3D_H
#define POISSON3D_H

#include <stdlib.h>

#define ALPHA 10E+5
#define EPS   10E-8

#define ITERS_MAX 100

#define phi(x, y, z) \
    ((x) * (x) + (y) * (y) + (z) * (z))

#define rho(x, y, z) \
    (6 - ALPHA * phi(x, y, z))

typedef struct Point {
    size_t x;
    size_t y;
    size_t z;
} Point;

double calculateEquation(Point size, Point h);

#endif