#include "server.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>

#include "list.h"

#define PTHREAD_PASS 0
#define SOCK_ERROR  -1

#define pthreadErrorHandle(test_code, pass_code, err_msg) do {		\
	if (test_code != pass_code) {									\
		fprintf(stderr, "%s: %s\n", err_msg, strerror(test_code));	\
		exit(EXIT_FAILURE);											\
	}																\
} while (0)

#define sockErrorHandle(test_code, err_code, serv_sock, err_msg) do {	\
	if (test_code == err_code) {										\
		fprintf(stderr, "%s: %s\n", err_msg, strerror(test_code));		\
		close(serv_sock);												\
		exit(EXIT_FAILURE);												\
	}																	\
} while (0)

void clientClose(int client_fd) {
	shutdown(client_fd, SHUT_WR);
	//recv(client_fd, NULL, 1, MSG_PEEK | MSG_DONTWAIT);
	close(client_fd);
}

void clientWrite(int client_fd, char* response, size_t response_size) {
	write(client_fd, response, response_size);
}

void serverStart(pthread_func thread_func, size_t stack_size) {
	struct sockaddr_in serv_addr;
 
	int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	sockErrorHandle(serv_sock, SOCK_ERROR, serv_sock, "Socket open error"); 

 	//char opt = 1;
	//setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT | SO_KEEPALIVE, &opt, sizeof(opt));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERVER_PORT);
 
	int err = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
 	sockErrorHandle(err, SOCK_ERROR, serv_sock, "Bind error");
	
	err = listen(serv_sock, DEFAULT_CLIENT_COUNT); 
 	sockErrorHandle(err, SOCK_ERROR, serv_sock, "Listen error");

	pthread_attr_t thread_attr;
	err = pthread_attr_init(&thread_attr);
	pthreadErrorHandle(err, PTHREAD_PASS, "Cannot create thread attribute");

	err = pthread_attr_setstacksize(&thread_attr, stack_size);
	pthreadErrorHandle(err, PTHREAD_PASS, "Setting thread stack size failed");

	List* threads_list = NULL;
	listInit(&threads_list, sizeof(pthread_t), NULL);

	struct sockaddr_in client_addr;
	socklen_t sock_len = sizeof(client_addr);

	pid_t serv_pid = getpid();

	uint32_t connection_count = 0;
	while (true) {
		int client_fd = accept(serv_sock, (struct sockaddr*)&client_addr, &sock_len);
		if (SOCK_ERROR < client_fd) {
			printf("Server [%d]: got connection - %u\n", serv_pid, ++connection_count);

			pthread_t thread;
			listPushBack(threads_list, (void*)&thread);	

			ThreadParam thread_param = (ThreadParam){connection_count, client_fd};

			err = pthread_create((pthread_t*)listBack(threads_list), &thread_attr, thread_func, (void*)&thread_param);
			pthreadErrorHandle(err, PTHREAD_PASS, "Cannot create a thread");
		}
		
		if (!listIsEmpty(threads_list)) {
			ListNode* threads_list_iter = threads_list->head;
			do {
				if (pthread_kill(*(pthread_t*)threads_list_iter->data, 0)) {
					pthread_join(*(pthread_t*)threads_list_iter->data, NULL);
					listDeleteNode(threads_list, threads_list_iter);
				}

				threads_list_iter = threads_list_iter->next;
			} while (threads_list_iter);
		}	
	}

	listFree(&threads_list);
	close(serv_sock);
}