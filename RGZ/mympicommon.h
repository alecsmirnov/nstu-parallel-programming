#ifndef MYMPICOMMON_H
#define MYMPICOMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>

struct MyMPICommon {
    struct sockaddr_in serv_addr;
    int serv_sock;

    const char* hostname;
    uint16_t port;

    int rank;
    int num_procs;
} mpi_common;

typedef struct MyMPIDataHeader {
    int src;
    int tag;
} MyMPIDataHeader;

#endif