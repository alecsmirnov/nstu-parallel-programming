#include "mympi.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

/* ARGS */
#define ARGS_COUNT 5

#define ARG_HOST 1
#define ARG_PORT 2
#define ARG_RANK 3
#define ARG_SIZE 4

/* Server */
#define CLIENT_COUNT 128

#define SOCK_ERR -1
#define SOCK_PASS 0

/* myMPI */
#define ROOT_RANK 0

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

enum DataType {
    DT_MSG,
    DT_BLOCK
};

typedef struct DataHeader {
    int src;
    int tag;

    uint8_t data_type;
} DataHeader;

struct MyMPIData {
    struct sockaddr_in serv_addr;
    int serv_sock;

    int block_size;
};

struct MyMPIComm {
    const char* hostname;
    uint16_t port;

    int rank;
    int size;
};

static void dataSend(int sock, void* data, size_t data_size) {
    const char* data_ptr = (const char*)data;

    size_t send_size = 0;
    while (send_size < data_size) {
        int bytes_send = send(sock, data_ptr, data_size, 0);
        if (bytes_send == SOCK_ERR)
            throwErr("Error: send data header to dest!"); 

        data_ptr += bytes_send;
        send_size += bytes_send;
    }
}

static void dataRecv(int sock, void* data, size_t data_size) {
    char* data_ptr = (char*)data;

    size_t recv_size = 0;
    while (recv_size < data_size) {
        int bytes_recv = recv(sock, data_ptr, data_size, 0);
        if (bytes_recv == SOCK_ERR)
            throwErr("Error: recv data from src!");

        data_ptr += bytes_recv;
        recv_size += bytes_recv;
    }
}

static void blockSend(int dest) {
    int send_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (send_sock == SOCK_ERR) 
        throwErr("Error: barrier send socket open!"); 

    struct sockaddr_in send_addr = mympi_data->serv_addr;    
    send_addr.sin_port = htons(mympi_comm->port + dest);

    while (connect(send_sock, (struct sockaddr*)&send_addr, sizeof(send_addr)) != SOCK_PASS);

    DataHeader data_header = (DataHeader){-1, -1, DT_BLOCK};
    dataSend(send_sock, (void*)&data_header, sizeof(DataHeader));

    close(send_sock);
}

static void blockRecv() {
    DataHeader data_header;

    int recv_sock = 0;
    do {
        struct sockaddr_in recv_addr;
        socklen_t recv_len = sizeof(recv_addr);

        recv_sock = accept(mympi_data->serv_sock, (struct sockaddr*)&recv_addr, (socklen_t*)&recv_len);
        if (recv_sock == SOCK_ERR)
            throwErr("Error: comm recv accept!");

        dataRecv(recv_sock, (void*)&data_header, sizeof(DataHeader));
    } while (data_header.data_type != DT_BLOCK);

    close(recv_sock);
}

static void blockAdd() {
    ++mympi_data->block_size;
}

static void blockClear() {
    mympi_data->block_size = 0;
}

void myMPIInit(int* argc, char** argv[]) {
    if (*argc < ARGS_COUNT) 
        throwErr("Wrong number of arguments!\n"
                 "Enter: <hostname> <port> <rank> <number of processes>\n");

    mympi_data = (struct MyMPIData*)malloc(sizeof(struct MyMPIData));
    mympi_comm = (struct MyMPIComm*)malloc(sizeof(struct MyMPIComm));

    mympi_comm->hostname = (*argv)[ARG_HOST];
    mympi_comm->port = atoi((*argv)[ARG_PORT]);
    mympi_comm->rank = atoi((*argv)[ARG_RANK]);
    mympi_comm->size = atoi((*argv)[ARG_SIZE]);

    mympi_data->block_size = 0;

    mympi_data->serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (mympi_data->serv_sock == SOCK_ERR)
        throwErr("Error: serv socket open!"); 

    struct hostent* hosten = gethostbyname(mympi_comm->hostname); 

    mympi_data->serv_addr.sin_family = AF_INET;
    mympi_data->serv_addr.sin_addr.s_addr = ((struct in_addr*)hosten->h_addr_list[0])->s_addr;
    mympi_data->serv_addr.sin_port = htons(mympi_comm->port + mympi_comm->rank);

    int err = 0;
    err = bind(mympi_data->serv_sock, (struct sockaddr*)&mympi_data->serv_addr, sizeof(mympi_data->serv_addr));
    if (err == SOCK_ERR)
        throwErr("Error: bind serv socket!");

    err = listen(mympi_data->serv_sock, CLIENT_COUNT); 
    if (err == SOCK_ERR)
        throwErr("Error: listen serv socket!");
}

void myMPIBarrier() {
    if (mympi_comm->rank == ROOT_RANK) {
        while (mympi_data->block_size < mympi_comm->size - 1) {
            blockRecv();
            blockAdd();
        }
        
        for (int dest = 0; dest < mympi_comm->size; ++dest)
            if (dest != ROOT_RANK)
                blockSend(dest);

        blockClear();
    }
    else {
        blockSend(ROOT_RANK);
        blockRecv();
    }
}

void myMPIFinalize() {
    myMPIBarrier();

    close(mympi_data->serv_sock);

    free(mympi_data);
    free(mympi_comm);
}

void myMPICommRank(int* rank) {
    if (rank == NULL) 
        throwErr("Error: rank null ptr!"); 

    *rank = mympi_comm->rank;
}

void myMPICommSize(int* size) {
    if (size == NULL)
        throwErr("Error: size null ptr!"); 

    *size = mympi_comm->size;
}

void myMPISend(void* data, size_t count, size_t datatype, int dest, int tag) {
    int send_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (send_sock == SOCK_ERR) 
        throwErr("Error: send socket open!"); 

    struct sockaddr_in send_addr = mympi_data->serv_addr;    
    send_addr.sin_port = htons(mympi_comm->port + dest);

    while (connect(send_sock, (struct sockaddr*)&send_addr, sizeof(send_addr)) != SOCK_PASS);

    DataHeader data_header = (DataHeader){mympi_comm->rank, tag, DT_MSG};

    dataSend(send_sock, (void*)&data_header, sizeof(DataHeader));
    dataSend(send_sock, data, count * datatype);

    close(send_sock);
}

void myMPIRecv(void* data, size_t count, size_t datatype, int src, int tag) {
    DataHeader data_header;

    int recv_sock = 0;
    do {
        struct sockaddr_in recv_addr;
        socklen_t recv_len = sizeof(recv_addr);

        recv_sock = accept(mympi_data->serv_sock, (struct sockaddr*)&recv_addr, (socklen_t*)&recv_len);
        if (recv_sock == SOCK_ERR) 
            throwErr("Error: mpi recv accept!"); 

        dataRecv(recv_sock, (void*)&data_header, sizeof(DataHeader));

        switch (data_header.data_type) {
            case DT_MSG:   dataRecv(recv_sock, data, count * datatype); break;
            case DT_BLOCK: blockAdd();                                  break;
            default:       throwErr("Error: unrecognized data type!");
        }
    } while (data_header.src != src || data_header.tag != tag);

    close(recv_sock);
}