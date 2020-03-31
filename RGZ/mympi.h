#ifndef MYMPI_H
#define MYMPI_H

#include "mympicommon.h"

void myMPIInit(int* argc, char** argv[]);
void myMPIFinalize();

void myMPICommRank(int* rank);
void myMPICommSize(int* num_procs);

void myMPISend(void* data, size_t count, size_t datatype, int dest, int tag);
void myMPIRecv(void* data, size_t count, size_t datatype, int src, int tag);

void myMPIBarrier();

#endif