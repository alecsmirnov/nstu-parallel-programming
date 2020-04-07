#include <stdio.h>
#include <time.h>
#include <mpi.h>

#include "poisson3d.h"


#define BILLION 1.0E+9

#define clocktimeDifference(start, stop)            \
    1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

int main(int argc, char* argv[]) {
    size_t grid_size = 12;

    Point D = (Point){grid_size, grid_size, grid_size};
    Point N = (Point){grid_size, grid_size, grid_size};
    DPoint p0 = (DPoint){0, 0, 0};

    int rank = 0;
    int size = 1;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Barrier(MPI_COMM_WORLD);
    struct timespec start, stop;
	clock_gettime(CLOCK_MONOTONIC, &start);

    P3DResult res;
    solveEquation(D, N, p0, &res);

    MPI_Barrier(MPI_COMM_WORLD);
    clock_gettime(CLOCK_MONOTONIC, &stop);

    if (rank == 0) {
        printf("Result: %.15lf\n", res.result);
        printf("Iters:  %zu\n", res.iters);

        printf("Elapsed time: %lf\n", clocktimeDifference(start, stop));
    }

    MPI_Finalize();

    return 0;
}