#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    printf("[%d]\n", rank);

    size_t data_size = 12;
    char* data = (char*)malloc(sizeof(char)* data_size);
    
    if (rank == 0) {
        strcpy(data, "hello proc0");

        MPI_Send(data, data_size, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
    }
    sleep(1);
    if (rank == 1) {
        strcpy(data, "hello proc1");

        MPI_Send(data, data_size, MPI_CHAR, 2, 0, MPI_COMM_WORLD);
    }
    sleep(2);

    //else {
    if (rank == 2) {        
        MPI_Status status;
        MPI_Recv(data, data_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);

        printf("Process[%d]: %s\n", rank, data);

        MPI_Recv(data, data_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, &status);

        printf("Process[%d]: %s\n", rank, data);
    }

    MPI_Finalize();

    return 0;
}