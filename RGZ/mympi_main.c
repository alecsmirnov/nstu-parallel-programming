#include <stdio.h>
#include <string.h>

#include "mympi.h"

int main(int argc, char* argv[]) {

    int rank;
    int num_procs;

    myMPIInit(&argc, &argv);

    myMPICommRank(&rank);
    myMPICommSize(&num_procs);

    printf("[%d] of %d\n", rank, num_procs);

    size_t data_size = 12;
    char* data = (char*)malloc(sizeof(char)* data_size);
    
    if (rank == 0) {
        strcpy(data, "hello world");

        myMPISend((void*)data, data_size, sizeof(char), 1);
    }
    //else {
    if (rank == 1) {        
        myMPIRecv((void*)data, data_size, sizeof(char), 0);

        printf("Process[%d]: %s\n", rank, data);
    }

    myMPIFinalize();

    return 0;
}
