#include "poisson3d.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define gridAccess(grid, size, i, j, k) \
    ((grid)[(i) * size.y * size.z + (j) * size.z + (k)])

#define max(x, y) ((x) > (y) ? (x) : (y))

static bool isBoundary(Point size, size_t i, size_t j, size_t k) {
    return i == 0 || i == size.x - 1 || 
           j == 0 || j == size.y - 1 || 
           k == 0 || k == size.z - 1;
}

static void setBoundary(double* grid, Point size, Point h) {
    for (size_t i = 0; i != size.x; ++i)
        for (size_t j = 0; j != size.y; ++j)
            for (size_t k = 0; k != size.z; ++k) {
                double x = 0;
                double y = 0;
                double z = 0;

                if (isBoundary(size, i, j, k)) {
                    x = i * h.x;
                    y = j * h.y;
                    z = k * h.z;
                }

                gridAccess(grid, size, i, j, k) = phi(x, y, z);
            }
}

static double jacobi(double* grid, Point size, Point h, size_t i, size_t j, size_t k) {
    double hx2 = h.x * h.x;
    double hy2 = h.y * h.y;
    double hz2 = h.z * h.z;

    double phix = (gridAccess(grid, size, i - 1, j, k) + 
                   gridAccess(grid, size, i + 1, j, k)) / hx2;
    double phiy = (gridAccess(grid, size, i, j - 1, k) + 
                   gridAccess(grid, size, i, j + 1, k)) / hy2;
    double phiz = (gridAccess(grid, size, i, j, k + 1) + 
                   gridAccess(grid, size, i, j, k + 1)) / hz2;

    return (phix + phiy + phiz - rho(i * h.x, j * h.y, k * h.z)) / 
           (2 / hx2 + 2 / hy2 + 2 / hz2 + ALPHA);
}

double calculateEquation(Point size, Point h) {
    double* grid = (double*)malloc(sizeof(double) * size.x * size.y * size.z);
    if (grid == NULL) 
        throwErr("Error: grid out of memmory!");

    double* new_grid = (double*)malloc(sizeof(double) * size.x * size.y * size.z);
    if (new_grid == NULL)
        throwErr("Error: new grid out of memmory!");
    
    setBoundary(grid, size, h);

    double jacobi_res = 0;
    size_t iters = 0;

    do {
        for (size_t i = 1; i != size.x - 1; ++i)
            for (size_t j = 1; j != size.y - 1; ++j)
                for (size_t k = 1; k != size.z - 1; ++k) {
                    gridAccess(new_grid, size, i, j, k) = jacobi(grid, size, h, i, j, k);
                    jacobi_res = max(jacobi_res, fabs(gridAccess(grid, size, i, j, k) - 
                                                      gridAccess(new_grid, size, i, j, k)));
                }

        memcpy(grid, new_grid, sizeof(double) * size.x * size.y * size.z);

        ++iters;
    } while (EPS < jacobi_res && iters < ITERS_MAX);

    free(grid);
    free(new_grid);

    return jacobi_res;
}