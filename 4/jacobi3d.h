#ifndef JACOBI3D_H
#define JACOBI3D_H

#include <stdlib.h>

#ifdef WITH_MPI
#include <mpi.h>
#endif

#define ALPHA 10E+5         // Параметр уравнения
#define EPS   10E-8         // Порог сходимости

#define ITERS_MAX 1000      // Кол-во итераций метода

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

typedef struct P3DResult {
    double result;
    size_t iters;
} P3DResult;

void solveEquation(Point D, Point N, DPoint p0, P3DResult* result);

#endif