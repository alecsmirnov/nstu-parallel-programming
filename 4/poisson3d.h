#ifndef POISSON3D_H
#define POISSON3D_H

#include <stdlib.h>

#define ALPHA 10E+5
#define EPS   10E-8

#define ITERS_MAX 100

typedef struct Point {
    size_t x;
    size_t y;
    size_t z;
} Point;

typedef struct DPoint {
    double x;
    double y;
    double z;
} DPoint;

typedef struct Grid {
    double* data;

    Point D;
    Point N;

    DPoint h;
    DPoint p0;
} Grid;

typedef struct P3DResult {
    double result;
    size_t iters;
} P3DResult;

void gridInit(Grid* grid, Point D, Point N, DPoint p0);

P3DResult solveEquation(Grid* grid);

#endif