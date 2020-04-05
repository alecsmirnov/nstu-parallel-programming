#include "poisson3d.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define max(x, y) \
    ((x) > (y) ? (x) : (y))

#define gridAccess(grid, D, i, j, k) \
    ((grid)[(i) * D.y * D.z + (j) * D.z + (k)])

#define phi(x, y, z) \
    ((x) * (x) + (y) * (y) + (z) * (z))

#define rho(x, y, z) \
    (6 - ALPHA * phi(x, y, z))

static bool isBoundary(Point D, size_t i, size_t j, size_t k) {
    return i == 0 || i == D.x - 1 || 
           j == 0 || j == D.y - 1 || 
           k == 0 || k == D.z - 1;
}

static void setBoundary(Grid* grid) {
    for (size_t i = 0; i != grid->D.x; ++i)
        for (size_t j = 0; j != grid->D.y; ++j)
            for (size_t k = 0; k != grid->D.z; ++k) {
                double x = 0;
                double y = 0;
                double z = 0;

                if (isBoundary(grid->D, i, j, k)) {
                    x = grid->p0.x + i * grid->h.x;
                    y = grid->p0.y + j * grid->h.y;
                    z = grid->p0.z + k * grid->h.z;
                }

                gridAccess(grid->data, grid->D, i, j, k) = phi(x, y, z);
            }
}

static double jacobi(Grid* grid, size_t i, size_t j, size_t k) {
    double hx2 = grid->h.x * grid->h.x;
    double hy2 = grid->h.y * grid->h.y;
    double hz2 = grid->h.z * grid->h.z;

    double phix = (gridAccess(grid->data, grid->D, i - 1, j, k) + 
                   gridAccess(grid->data, grid->D, i + 1, j, k)) / hx2;
    double phiy = (gridAccess(grid->data, grid->D, i, j - 1, k) + 
                   gridAccess(grid->data, grid->D, i, j + 1, k)) / hy2;
    double phiz = (gridAccess(grid->data, grid->D, i, j, k - 1) + 
                   gridAccess(grid->data, grid->D, i, j, k + 1)) / hz2;

    double x = grid->p0.x + i * grid->h.x;
    double y = grid->p0.y + j * grid->h.y;
    double z = grid->p0.z + k * grid->h.z;
    
    return (phix + phiy + phiz - rho(x, y, z)) / 
           (2 / hx2 + 2 / hy2 + 2 / hz2 + ALPHA);
}

void gridInit(Grid* grid, Point D, Point N, DPoint p0) {
    if (grid == NULL)
        throwErr("Error: grid null ptr!");

    grid->data = (double*)malloc(sizeof(double) * D.x * D.y * D.z);
    if (grid->data == NULL) 
        throwErr("Error: grid data out of memmory!");

    grid->D = D;
    grid->N = N;
    grid->h = (DPoint){1.0 * D.x / (N.x - 1), 1.0 * D.y / (N.y - 1), 1.0 * D.y / (N.y - 1)};
    grid->p0 = p0;
}

P3DResult solveEquation(Grid* grid) {
    double* new_data = (double*)malloc(sizeof(double) * grid->D.x * grid->D.y * grid->D.z);
    if (new_data == NULL)
        throwErr("Error: new grid out of memmory!");
    
    setBoundary(grid);
    memcpy(new_data, grid->data, sizeof(double) * grid->D.x * grid->D.y * grid->D.z);

    double result = 0;
    size_t iters = 0;

    do {
        result = 0;
        for (size_t i = 1; i != grid->D.x - 1; ++i)
            for (size_t j = 1; j != grid->D.y - 1; ++j)
                for (size_t k = 1; k != grid->D.z - 1; ++k) {
                    gridAccess(new_data, grid->D, i, j, k) = jacobi(grid, i, j, k);

                    result = max(result, fabs(gridAccess(grid->data, grid->D, i, j, k) - 
                                              gridAccess(new_data, grid->D, i, j, k)));
                }

        memcpy(grid->data, new_data, sizeof(double) * grid->D.x * grid->D.y * grid->D.z);

        ++iters;
    } while (EPS < result && iters < ITERS_MAX);

    free(new_data);

    return (P3DResult){result, iters};
}