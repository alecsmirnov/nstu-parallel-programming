#ifndef MYMPICOMMON_H
#define MYMPICOMMON_H

#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>

#define ROOT 0

typedef enum MyMPIMsgType {
    MMT_MSG,
    MMT_BLOCK
} MyMPIMsgType;

struct MyMPICommon {
    struct sockaddr_in serv_addr;
    int serv_sock;

    const char* hostname;
    uint16_t port;

    int rank;
    int size;

    int block_size;
} mpi_common;

typedef struct MyMPIDataHeader {
    int src;
    int tag;

    MyMPIMsgType msg_type;
} MyMPIDataHeader;

#endif