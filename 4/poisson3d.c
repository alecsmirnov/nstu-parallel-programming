#include "poisson3d.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <mpi.h>

#include <unistd.h>

#define ROOT_RANK 0

#define MIN_DX_SIZE 3

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define gridAccess(grid, D, i, j, k) \
    ((grid)[(i) * D.y * D.z + (j) * D.z + (k)])

#define phi(x, y, z) \
    ((x) * (x) + (y) * (y) + (z) * (z))

#define rho(x, y, z) \
    (6 - ALPHA * phi(x, y, z))

#define max(x, y) \
    ((x) > (y) ? (x) : (y))

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

static double jacobi(double* data, Point D, DPoint h, DPoint p0, size_t i, size_t j, size_t k) {
    double hx2 = h.x * h.x;
    double hy2 = h.y * h.y;
    double hz2 = h.z * h.z;

    double phix = (gridAccess(data, D, i - 1, j, k) + gridAccess(data, D, i + 1, j, k)) / hx2;
    double phiy = (gridAccess(data, D, i, j - 1, k) + gridAccess(data, D, i, j + 1, k)) / hy2;
    double phiz = (gridAccess(data, D, i, j, k - 1) + gridAccess(data, D, i, j, k + 1)) / hz2;

    double x = p0.x + i * h.x;
    double y = p0.y + j * h.y;
    double z = p0.z + k * h.z;

    return (phix + phiy + phiz - rho(x, y, z)) / (2 / hx2 + 2 / hy2 + 2 / hz2 + ALPHA);
}

void gridInit(Grid* grid, Point D, Point N, DPoint p0) {
    if (grid == NULL)
        throwErr("Error: grid null ptr!");

    grid->data = (double*)malloc(sizeof(double) * D.x * D.y * D.z);
    if (grid->data == NULL) 
        throwErr("Error: grid data out of memmory!");

    grid->D = D;
    grid->N = N;
    grid->h = (DPoint){1.0 * D.x / (N.x - 1), 1.0 * D.y / (N.y - 1), 1.0 * D.z / (N.z - 1)};
    grid->p0 = p0;

    setBoundary(grid);
}

void solveEquation(Point D, Point N, DPoint p0, P3DResult* result) {
    P3DResult root_result;

    int rank = ROOT_RANK;
    int size = 1;
    
    // need to check for MPI def
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Grid grid;
    gridInit(&grid, D, N, p0);

     // need to check for MPI def
    MPI_Request up_send;
    MPI_Request up_recv;
    MPI_Request down_send;
    MPI_Request down_recv;

    int* local_counts = NULL;
    int* offsets = NULL;

    local_counts = (int*)malloc(sizeof(int) * size);
    offsets = (int*)malloc(sizeof(int) * size);

    int chunk_size = D.x / size;
    int remainder = D.x % size;

    // Определение размеров блоков данных
    int shift = 0;
    for (int i = 0; i < size; ++i) {
        local_counts[i] = chunk_size + (i < remainder ? 1 : 0);

        offsets[i] = shift;
        shift += local_counts[i];
    }

    int borders = 0;
    int borders_offset = 0;

    borders = 2;
    borders_offset = 1;
    if (rank == 0 || rank == size - 1) {
        borders = 1;
        if (rank == 0)
            borders_offset = 0;
    }

    for (int i = 0; i < size; ++i) {
        offsets[i] *= D.y * D.z;
        local_counts[i] *= D.y * D.z;
    }

    borders *= D.z * D.y;
    borders_offset *= D.z * D.y;

    double* localArray = (double*)malloc(sizeof(double) * (local_counts[rank] + borders));

    MPI_Scatterv(grid.data, local_counts, offsets, MPI_DOUBLE, 
            localArray + borders_offset, local_counts[rank], MPI_DOUBLE, ROOT_RANK, MPI_COMM_WORLD);

    //--------------------------

    double* new_data = (double*)malloc(sizeof(double) * (local_counts[rank] + borders));
    if (new_data == NULL)
        throwErr("Error: new grid out of memmory!");

    memcpy(new_data, localArray, sizeof(double) * (local_counts[rank] + borders));

    double jacobi_result = 0;
    size_t iters = 0;
    do {
        jacobi_result = 0;

        if (rank < size - 1) {
            MPI_Isend(localArray + (local_counts[rank] + borders_offset - D.y * D.z), D.y * D.z, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &down_send);
            MPI_Irecv(localArray + (local_counts[rank] + borders_offset), D.y * D.z, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &down_recv);
        }

        if (0 < rank) {
            MPI_Isend(localArray + borders_offset, D.y * D.z, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &up_send);
            MPI_Irecv(localArray, D.y * D.z, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &up_recv);
        }

        // обработка до границ
        if (rank == 0) {
            size_t field_beg = 1;
            size_t field_end = (local_counts[rank] + borders) / (D.y * D.z) - 2;
            for (size_t i = field_beg; i != field_end; ++i)
                for (size_t j = 1; j != grid.D.y - 1; ++j)
                    for (size_t k = 1; k != grid.D.z - 1; ++k) {
                        gridAccess(new_data, grid.D, i, j, k) = jacobi(localArray, grid.D, grid.h, grid.p0, i, j, k);

                        jacobi_result = max(jacobi_result, fabs(gridAccess(localArray, grid.D, i, j, k) - gridAccess(new_data, grid.D, i, j, k)));
                    }
        }

        if (rank == size - 1) {
            size_t field_beg = 2;
            size_t field_end = (local_counts[rank] + borders) / (D.y * D.z) - 1;
            for (size_t i = field_beg; i != field_end; ++i)
                for (size_t j = 1; j != grid.D.y - 1; ++j)
                    for (size_t k = 1; k != grid.D.z - 1; ++k) {
                        gridAccess(new_data, grid.D, i, j, k) = jacobi(localArray, grid.D, grid.h, grid.p0, i, j, k);

                        jacobi_result = max(jacobi_result, fabs(gridAccess(localArray, grid.D, i, j, k) - gridAccess(new_data, grid.D, i, j, k)));
                    }
        }

        if (0 < rank && rank < size - 1) {
            for (size_t i = 2; i != (local_counts[rank] + borders) / (D.y * D.z) - 2; ++i)
                for (size_t j = 1; j != grid.D.y - 1; ++j)
                    for (size_t k = 1; k != grid.D.z - 1; ++k) {
                        gridAccess(new_data, grid.D, i, j, k) = jacobi(localArray, grid.D, grid.h, grid.p0, i, j, k);

                        jacobi_result = max(jacobi_result, fabs(gridAccess(localArray, grid.D, i, j, k) - gridAccess(new_data, grid.D, i, j, k)));
                    }
        }

        if (rank < size - 1) {
            MPI_Wait(&down_send, MPI_STATUSES_IGNORE);
            MPI_Wait(&down_recv, MPI_STATUSES_IGNORE);
        }

        if (0 < rank) {
            MPI_Wait(&up_send, MPI_STATUSES_IGNORE);
            MPI_Wait(&up_recv, MPI_STATUSES_IGNORE);
        }

        if (rank < size - 1) {
            size_t bottom = (local_counts[rank] + borders) / (D.y * D.z) - 2;
            for (size_t j = 1; j != grid.D.y - 1; ++j)
                for (size_t k = 1; k != grid.D.z - 1; ++k) {
                    gridAccess(new_data, grid.D, bottom, j, k) = jacobi(localArray, grid.D, grid.h, grid.p0, bottom, j, k);

                    jacobi_result = max(jacobi_result, fabs(gridAccess(localArray, grid.D, bottom, j, k) - gridAccess(new_data, grid.D, bottom, j, k)));
                }
        }

        if (0 < rank) {
            size_t upper = 1;
            for (size_t j = 1; j != grid.D.y - 1; ++j)
                for (size_t k = 1; k != grid.D.z - 1; ++k) {
                    gridAccess(new_data, grid.D, upper, j, k) = jacobi(localArray, grid.D, grid.h, grid.p0, upper, j, k);

                    jacobi_result = max(jacobi_result, fabs(gridAccess(localArray, grid.D, upper, j, k) - gridAccess(new_data, grid.D, upper, j, k)));
                }
        }

        MPI_Status status;
        if (rank == ROOT_RANK) {
            for (int i = 1; i < size; ++i) {
                double rank_result = 0;
                MPI_Recv(&rank_result, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);

                jacobi_result = max(jacobi_result, rank_result);
            }

            for (int i = 1; i < size; ++i)
                MPI_Send(&jacobi_result, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
        else {
            MPI_Send(&jacobi_result, 1, MPI_DOUBLE, ROOT_RANK, 0, MPI_COMM_WORLD);
            MPI_Recv(&jacobi_result, 1, MPI_DOUBLE, ROOT_RANK, 0, MPI_COMM_WORLD, &status);
        }

        memcpy(localArray, new_data, sizeof(double) * (local_counts[rank] + borders));
        
        ++iters;
    } while (EPS < jacobi_result && iters < ITERS_MAX);

    free(local_counts);
    free(offsets);

    free(new_data);

    free(grid.data);

    if (rank == ROOT_RANK) {
        *result = (P3DResult){jacobi_result, iters};
    }
}