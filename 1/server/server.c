#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "list.h"

#define SERVER_PORT 8080

#define CONNECTION_COUNT_MAX 6
#define CLIENT_COUNT_MAX 50

#define RESPONSE_LEN_MAX 512

#define ERROR -1

static const char* RESPONSE_TEMPLATE = "HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html; charset=UTF-8\r\n\r\n"
	"<!DOCTYPE html><html><head><title>Request</title>" 
	"<style>h1{text-align: center;}</style></head>"
	"<body><h1>Request number %u has been processed</h1></body></html>\r\n";

typedef struct ThreadParam {
	uint32_t request_num;
	int client_fd;
} ThreadParam;

static void* threadFunc(void* arg) {
	ThreadParam* thread_param = (ThreadParam*)arg;

	char response[RESPONSE_LEN_MAX];
	snprintf(response, sizeof(response), RESPONSE_TEMPLATE, thread_param->request_num);

	write(thread_param->client_fd, response, strlen(response));
	close(thread_param->client_fd);

	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	struct sockaddr_in serv_addr;
 
	int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sock < ERROR) {
		fprintf(stderr, "Can't open socket\n");

		close(serv_sock);
		exit(EXIT_FAILURE);
	}

 	char opt = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
 
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = htons(SERVER_PORT);
 
	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == ERROR) {
		fprintf(stderr, "Bind error\n");

		close(serv_sock);
		exit(EXIT_FAILURE);
	}
 
	if (listen(serv_sock, CLIENT_COUNT_MAX) == ERROR) {
		fprintf(stderr, "Listen error\n");

		close(serv_sock);
		exit(EXIT_FAILURE);
	}

	List* threads_list = NULL;
	listInit(&threads_list, sizeof(pthread_t), NULL);

	uint32_t connection_count = 0;
	while (connection_count != CONNECTION_COUNT_MAX) {
		struct sockaddr_in client_addr;
		socklen_t sock_len = sizeof(client_addr);

		int client_fd = accept(serv_sock, (struct sockaddr*)&client_addr, &sock_len);
		if (client_fd == ERROR) {
			fprintf(stderr, "Accept error\n");

			close(serv_sock);
			exit(EXIT_FAILURE);
		}

		printf("Got connection %u\n", ++connection_count);

		pthread_t thread;
		listPushBack(threads_list, (void*)&thread);	

		ThreadParam thread_param = (ThreadParam){connection_count, client_fd};
		pthread_create((pthread_t*)listBack(threads_list), NULL, threadFunc, (void*)&thread_param);

		ListNode* threads_list_iter = threads_list->head;
		do {
			if (pthread_kill(*(pthread_t*)threads_list_iter->data, 0) != 0) {
				pthread_join(*(pthread_t*)threads_list_iter->data, NULL);
				listDeleteNode(threads_list, threads_list_iter);
			}

			threads_list_iter = threads_list_iter->next;
		} while (threads_list_iter);
	}

	listFree(&threads_list);
	close(serv_sock);

	return 0;
}