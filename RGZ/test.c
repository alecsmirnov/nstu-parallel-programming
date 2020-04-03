#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define ROOT_RANK 0

/* Функция отлова ошибок */
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define matAccess(mat, m, i, j) \
    ((mat)[(i) * (m) + (j)])

static double* arrayCreate(size_t size) {
    double* A = (double*)malloc(sizeof(double) * size);
    if (A == NULL)
        throwErr("Error: res out of memmory!");

    return A;
}

static void arrayRandInit(double* A, size_t size) {
    for (size_t i = 0; i < size; ++i)
        A[i] = rand() % size;
}

static void matVecMult(double* res, double* mat, double* vec, size_t n, size_t m) {
    if (res == NULL)
        throwErr("Error: res null ptr!");

    for (size_t i = 0; i < n; ++i) {
        res[i] = 0;

        for (size_t j = 0; j < m; ++j)
            res[i] += matAccess(mat, m, i, j) * vec[j];
    }
}

void readData(double* mat, double* vec, size_t* n, size_t* m) {
    
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int tag  = 0;
    int rank = 0;
    int size = 1;

    size_t  root_n = 0;
    size_t  root_m = 0;
    double* root_mat = NULL;
    double* root_res = NULL;

    size_t n = 0;
    size_t m = 0;

    double* mat = NULL;
    double* vec = NULL;
    double* res = NULL;

    MPI_Status status;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == ROOT_RANK) {
        // read data begin
        root_n = 3;
        root_m = m = 3;

        root_mat = arrayCreate(root_n * root_m);
        arrayRandInit(root_mat, root_n * root_m);

        vec = arrayCreate(root_m);
        arrayRandInit(vec, root_m);
        // read data end

        for (uint8_t i = 1; i < size; ++i) {
            MPI_Send(&root_m, 1, MPI_UNSIGNED_LONG_LONG, i, tag, MPI_COMM_WORLD);
            MPI_Send(vec, root_m, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
        }
        
        size_t chunk_size = root_n / size;
        uint8_t remainder = root_n % size;

        size_t shift = 0;
        for (uint8_t i = 0; i < size; ++i) {
            size_t row_size = chunk_size + (i < remainder ? 1 : 0);

            if (i != ROOT_RANK) {
                MPI_Send(&row_size, 1, MPI_UNSIGNED_LONG_LONG, i, tag, MPI_COMM_WORLD);
                MPI_Send(mat + shift, row_size * root_m, MPI_DOUBLE, i, tag, MPI_COMM_WORLD);
            }
            else {
                n = row_size;
                mat = root_mat;
            }
            
            shift += row_size * root_m;
        }
    }
    else {
        MPI_Recv(&m, 1, MPI_UNSIGNED_LONG_LONG, ROOT_RANK, tag, MPI_COMM_WORLD, &status);
        vec = arrayCreate(m);
        MPI_Recv(vec, m, MPI_DOUBLE, ROOT_RANK, tag, MPI_COMM_WORLD, &status);

        MPI_Recv(&n, 1, MPI_UNSIGNED_LONG_LONG, ROOT_RANK, tag, MPI_COMM_WORLD, &status);
        mat = arrayCreate(n * m);
        MPI_Recv(mat, n * m, MPI_DOUBLE, ROOT_RANK, tag, MPI_COMM_WORLD, &status);

        root_n = n;
    }
    
    res = arrayCreate(root_n);
    matVecMult(res, mat, vec, n, m);

    if (rank == ROOT_RANK) {
        size_t shift = n;
        for (uint8_t i = 1; i < size; ++i) {
            MPI_Recv(&n, 1, MPI_UNSIGNED_LONG_LONG, i, tag, MPI_COMM_WORLD, &status);
            MPI_Recv(res + shift, n, MPI_DOUBLE, i, tag, MPI_COMM_WORLD, &status);

            shift += n;
        }
    }
    else {
        MPI_Send(&n, 1, MPI_UNSIGNED_LONG_LONG, ROOT_RANK, tag, MPI_COMM_WORLD);
        MPI_Send(res, n, MPI_DOUBLE, ROOT_RANK, tag, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == ROOT_RANK) {
        printf("matrix:\n");
        for (size_t i = 0; i < root_n; ++i) {
            for (size_t j = 0; j < root_m; ++j)
                printf("%g ", matAccess(root_mat, root_m, i, j));
            printf("\n");
        }

        printf("vector:\n");
        for (size_t i = 0; i < m; ++i)
            printf("%g ", vec[i]);
        printf("\n");

        printf("result:\n");
        for (size_t i = 0; i < root_n; ++i)
            printf("%g ", res[i]);
        printf("\n");
    }

    free(mat);
    free(vec);
    free(res);

    MPI_Finalize();

    return 0;
}