#include <stdio.h>

#include "poisson3d.h"

int main(int argc, char* argv[]) {
    size_t size = 4;

    Point D = (Point){size, size, size};
    Point N = (Point){size, size, size};
    Point p0 = (Point){0, 0, 0};

    Grid grid;
    gridInit(&grid, D, N, p0);

    P3DResult res = solveEquation(&grid);

    printf("Result: %lf\n", res.result);
    printf("Iters:  %zu\n", res.iters);

    free(grid.data);

    return 0;
}