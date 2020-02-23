#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#define SERVER_PORT 8080

#define KB 1024
#define MB 1024 * KB

#define DEFAULT_STACK_SIZE   2 * MB
#define DEFAULT_CLIENT_COUNT 128

typedef void* (*pthread_func)(void*);

typedef struct ThreadParam {
	uint32_t request_num;
	int client_fd;
} ThreadParam;

void clientWrite(int client_fd, char* response, size_t response_size);
void clientClose(int client_fd);

void serverStart(pthread_func thread_func, size_t stack_size, size_t clear_pull);

#endif