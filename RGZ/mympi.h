#ifndef MYMPI_H
#define MYMPI_H

#include <stdint.h>
#include <stddef.h>

/* myMPI данные (данные одного процесса) */
struct MyMPIData* mympi_data;
struct MyMPIComm* mympi_comm;

void myMPIInit(int* argc, char** argv[]);
void myMPIBarrier();
void myMPIFinalize();

void myMPICommRank(int* rank);
void myMPICommSize(int* size);
void myMPICommPort(uint16_t* port);

void myMPISend(void* data, size_t count, size_t data_type, int dest, int tag);
void myMPIRecv(void* data, size_t count, size_t data_type, int src, int tag);

#endif