#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mympi.h"

int main(int argc, char* argv[]) {
    int rank;
    int size;

    myMPIInit(&argc, &argv);

    myMPICommRank(&rank);
    myMPICommSize(&size);

    myMPIBarrier();

    //printf("%d of %d\n", rank, num_procs);
    
    size_t data_size = 12;
    char* data = (char*)malloc(sizeof(char)* data_size);
    
    if (rank == 0) {
        strcpy(data, "hello proc0");

        myMPISend((void*)data, data_size, sizeof(char), 2, 0);
        myMPISend((void*)data, data_size, sizeof(char), 1, 0);
    }

    if (rank == 1) {
        myMPIRecv((void*)data, data_size, sizeof(char), 0, 0);

        printf("Process[%d]: %s\n", rank, data);

        strcpy(data, "hello proc1");

        myMPISend((void*)data, data_size, sizeof(char), 2, 0);
    }

    if (rank == 2) {        
        myMPIRecv((void*)data, data_size, sizeof(char), 1, 0);

        printf("Process[%d]: %s\n", rank, data);
    }

    free(data);
    
    myMPIBarrier();
    
    myMPIFinalize();

    return 0;
}
