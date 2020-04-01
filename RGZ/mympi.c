#include "mympi.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#include <fcntl.h>
#include <errno.h>

#define MPI_ARGS_COUNT 5

#define DEFAULT_CLIENT_COUNT 128

#define SOCK_ERR -1
#define SOCK_PASS 0

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

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

static void addBlock() {
    ++mpi_common.block_size;
}

static void clearBlockSize() {
    mpi_common.block_size = 0;
}

void myMPIInit(int* argc, char** argv[]) {
    if (*argc < MPI_ARGS_COUNT) 
        throwErr("Wrong number of arguments!\n"
                 "Enter: <hostname> <port> <rank> <number of processes>\n");

    int err = 0;

    mpi_common.hostname = (*argv)[1];
    mpi_common.port = atoi((*argv)[2]);
    mpi_common.rank = atoi((*argv)[3]);
    mpi_common.size = atoi((*argv)[4]);

    mpi_common.serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (mpi_common.serv_sock == SOCK_ERR)
		throwErr("Error: serv socket open!"); 

 	char opt = 1;
	setsockopt(mpi_common.serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    struct hostent* hosten = gethostbyname(mpi_common.hostname); 

	mpi_common.serv_addr.sin_family = AF_INET;
	mpi_common.serv_addr.sin_addr.s_addr = ((struct in_addr*)hosten->h_addr_list[0])->s_addr;
	mpi_common.serv_addr.sin_port = htons(mpi_common.port + mpi_common.rank);

	err = bind(mpi_common.serv_sock, (struct sockaddr*)&mpi_common.serv_addr, sizeof(mpi_common.serv_addr));
 	if (err == SOCK_ERR)
		throwErr("Error: bind serv socket!");
	
	err = listen(mpi_common.serv_sock, DEFAULT_CLIENT_COUNT); 
 	if (err == SOCK_ERR)
		throwErr("Error: listen serv socket!");

    clearBlockSize();
}

static void sendBlockStatus(int dest) {
    int send_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (send_sock == SOCK_ERR) 
    	throwErr("Error: barrier send socket open!"); 

    int rank;
    myMPICommRank(&rank);

    struct sockaddr_in send_addr = mpi_common.serv_addr;    
    send_addr.sin_port = htons(mpi_common.port + dest);

    while (connect(send_sock, (struct sockaddr*)&send_addr, sizeof(send_addr)) != SOCK_PASS);

    MyMPIDataHeader data_header = (MyMPIDataHeader){rank, -1, MMT_BLOCK};
    dataSend(send_sock, (void*)&data_header, sizeof(MyMPIDataHeader));

    close(send_sock);
}

static void recvBlockStatus() {
    struct sockaddr_in recv_addr;
    socklen_t recv_len = sizeof(recv_addr);

    int recv_sock = accept(mpi_common.serv_sock, (struct sockaddr*)&recv_addr, (socklen_t*)&recv_len);
    
    MyMPIDataHeader data_header;
    
    do {
        dataRecv(recv_sock, (void*)&data_header, sizeof(MyMPIDataHeader));
    } while (data_header.msg_type != MMT_BLOCK);

    close(recv_sock);
}

void myMPIBarrier() {
    int rank;
    int size;

    myMPICommRank(&rank);
    myMPICommSize(&size);

    if (rank == ROOT) {
        while (mpi_common.block_size < size - 1) {
            recvBlockStatus();
            addBlock();
        }
        
        for (int i = 0; i != size; ++i)
            if (i != ROOT)
                sendBlockStatus(i);

        clearBlockSize();
    }
    else {
        sendBlockStatus(ROOT);
        recvBlockStatus();
    }
}

void myMPIFinalize() {
    myMPIBarrier();

    close(mpi_common.serv_sock);
}

void myMPICommRank(int* rank) {
    if (rank == NULL) 
        throwErr("Error: rank null ptr!"); 

    *rank = mpi_common.rank;
}

void myMPICommSize(int* size) {
    if (size == NULL)
        throwErr("Error: size null ptr!"); 

    *size = mpi_common.size;
}

void myMPISend(void* data, size_t count, size_t datatype, int dest, int tag) {
    int err = 0;

    int send_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (send_sock == SOCK_ERR) 
    	throwErr("Error: send socket open!"); 

    struct sockaddr_in send_addr = mpi_common.serv_addr;    
    send_addr.sin_port = htons(mpi_common.port + dest);

    while (connect(send_sock, (struct sockaddr*)&send_addr, sizeof(send_addr)) != SOCK_PASS);

    int src = 0;
    myMPICommRank(&src);
    MyMPIDataHeader data_header = (MyMPIDataHeader){src, tag, MMT_MSG};

    dataSend(send_sock, (void*)&data_header, sizeof(MyMPIDataHeader));
    dataSend(send_sock, data, count * datatype);

    close(send_sock);
}

void myMPIRecv(void* data, size_t count, size_t datatype, int src, int tag) {
    struct sockaddr_in recv_addr;
    socklen_t recv_len = sizeof(recv_addr);

    MyMPIDataHeader data_header;
    void* recv_data = malloc(count * datatype);

    int recv_sock = 0;
    do {
        recv_sock = accept(mpi_common.serv_sock, (struct sockaddr*)&recv_addr, (socklen_t*)&recv_len);

        dataRecv(recv_sock, (void*)&data_header, sizeof(MyMPIDataHeader));

        if (data_header.msg_type == MMT_MSG)
            dataRecv(recv_sock, recv_data, count * datatype);
        else
            addBlock();
    } while (data_header.src != src || data_header.tag != tag);

    memcpy(data, recv_data, count * datatype);
    free(recv_data);

    close(recv_sock);
}