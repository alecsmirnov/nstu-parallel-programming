#ifndef MYMPI_H
#define MYMPI_H

#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>

#define DEFAULT_CLIENT_COUNT 128

struct MyMPICommon {
    struct sockaddr_in serv_addr;
    int serv_sock;

    const char* hostname;
    uint16_t port;

    int rank;
    int num_procs;
} mpi_common;

typedef struct MyMPIData {
    uint8_t rank;
    int tag;

    void* data;
    size_t count;
    size_t datatype;
} MyMPIData;

void myMPIInit(int* argc, char** argv[]);
void myMPIFinalize();

void myMPICommRank(int* rank);
void myMPICommSize(int* num_procs);

void myMPISend(void* data, size_t count, size_t datatype, uint8_t dest);
void myMPIRecv(void* data, size_t count, size_t datatype, uint8_t src);

#endif