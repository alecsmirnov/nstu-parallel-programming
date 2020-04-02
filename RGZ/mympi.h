#ifndef MYMPI_H
#define MYMPI_H

#include <stddef.h>

/* myMPI данные (данные одного процесса) */
struct MyMPIData* mympi_data;
struct MyMPIComm* mympi_comm;

void myMPIInit(int* argc, char** argv[]);
void myMPIBarrier();
void myMPIFinalize();

void myMPICommRank(int* rank);
void myMPICommSize(int* size);

void myMPISend(void* data, size_t count, size_t datatype, int dest, int tag);
void myMPIRecv(void* data, size_t count, size_t datatype, int src, int tag);

#endif