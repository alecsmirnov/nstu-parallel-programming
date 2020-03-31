#include "mympi.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>

#define MPI_ARGS_COUNT 5

#define SOCK_ERR -1
#define SOCK_PASS 0

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

void myMPIInit(int* argc, char** argv[]) {
    if (*argc < MPI_ARGS_COUNT) 
        throwErr("Wrong number of arguments!\n"
                 "Enter: <hostname> <port> <rank> <number of processes>\n");

    int err = 0;

    mpi_common.hostname = (*argv)[1];
    mpi_common.port = atoi((*argv)[2]);
    mpi_common.rank = atoi((*argv)[3]);
    mpi_common.num_procs = atoi((*argv)[4]);

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
}

void myMPIFinalize() {
    close(mpi_common.serv_sock);
}

void myMPICommRank(int* rank) {
    if (rank == NULL) 
        throwErr("Error: rank null ptr!"); 

    *rank = mpi_common.rank;
}

void myMPICommSize(int* num_procs) {
    if (num_procs == NULL)
        throwErr("Error: num_procs null ptr!"); 

    *num_procs = mpi_common.num_procs;
}

void myMPISend(void* data, size_t count, size_t datatype, uint8_t dest) {
    int err = 0;

    int send_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (send_sock == SOCK_ERR) 
    	throwErr("Error: send socket open!"); 

    struct sockaddr_in send_addr = mpi_common.serv_addr;    
    send_addr.sin_port = htons(mpi_common.port + dest);

    while (connect(send_sock, (struct sockaddr*) &send_addr, sizeof(send_addr)) != SOCK_PASS);

    err = send(send_sock, data, count * datatype, 0);
    if (err == SOCK_ERR)
        throwErr("Error: send data to dest!"); 

    close(send_sock);
}

void myMPIRecv(void* data, size_t count, size_t datatype, uint8_t src) {
    int err = 0;

    struct sockaddr_in recv_addr;
    socklen_t recv_len = sizeof(recv_addr);

    int recv_sock = accept(mpi_common.serv_sock, (struct sockaddr*)&recv_addr, (socklen_t*)&recv_len);

    err = read(recv_sock, data, count * datatype);
    if (err == SOCK_ERR)
        throwErr("Error: recv data from src!"); 

    close(recv_sock);
}