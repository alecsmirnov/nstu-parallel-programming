#include <stdio.h>
#include <time.h>

#include "poisson3d.h"

#define ARGS_COUNT 10

#define BILLION 1.0E+9

#define clocktimeDifference(start, stop)            \
    1.0 * (stop.tv_sec - start.tv_sec) +            \
    1.0 * (stop.tv_nsec - start.tv_nsec) / BILLION

int main(int argc, char* argv[]) {
    int rank = 0;
    int size = 1;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    if (argc < ARGS_COUNT) {
        if (rank == 0) {
            fprintf(stderr, "Wrong number of arguments!\n");
            fprintf(stderr, "Enter:\n<D.x> <D.y> <D.z>\n");
            fprintf(stderr, "<N.x> <N.y> <N.z>\n");
            fprintf(stderr, "<p0.x> <p0.y> <p0.z>\n");
        }

        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    Point D = (Point){atoi(argv[1]), atoi(argv[2]), atoi(argv[3])};
    Point N = (Point){atoi(argv[4]), atoi(argv[5]), atoi(argv[6])};
    DPoint p0 = (DPoint){atof(argv[7]), atof(argv[8]), atof(argv[9])};

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