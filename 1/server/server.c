#include "server.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCK_ERR -1

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

// Проверка условия очистки данных: 0 - не чистить, 
// n - чистить через каждые n записей
#define clearCheck(rec_num, clear_pull) \
	(clear_pull == 0 ? 0 : rec_num % clear_pull == 0)

// Все параметры потока
typedef struct ThreadInfo {
	// Открытые параметры
	uint32_t request_num;		// Номер запроса
	int client;					// Дескриптор клиента

	// Закрытый параметр
	pthread_t thread;			// Идентификатор потока
} ThreadInfo;

// Отправить ответ клиенту
void clientWrite(int client, char* response, size_t response_size) {
	if (write(client, response, response_size) != response_size)
		fprintf(stderr, "Client write error!\n");
}

// Закрытие соединения с клиентом
void clientClose(int client) {
	shutdown(client, SHUT_WR);
	recv(client, NULL, 1, MSG_PEEK | MSG_DONTWAIT);
	close(client);
}

// Запуск сервера с указанной функцией, размером стека и параметром очистки
// (Параметр очистки: 0 - не чистить, n - чистить через каждые n записей)
void serverStart(pthread_func thread_func, size_t stack_size, size_t clear_pull) {
	struct sockaddr_in serv_addr;
	int err = 0;
 
	int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sock == SOCK_ERR)
		throwErr("Socket open error!"); 

 	char opt = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERVER_PORT);
 
	err = bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
 	if (err == SOCK_ERR)
		throwErr("Bind error!");
	
	err = listen(serv_sock, DEFAULT_CLIENT_COUNT); 
 	if (err == SOCK_ERR)
		throwErr("Listen error!");

 	struct sockaddr_in client_addr;
	socklen_t sock_len = sizeof(client_addr);

	pthread_attr_t thread_attr;
	err = pthread_attr_init(&thread_attr);
	if (err != 0)
		throwErr("Error: cannot create thread attribute!");

	err = pthread_attr_setstacksize(&thread_attr, stack_size);
	if (err != 0)
		throwErr("Error: setting thread stack size failed!");

	// Инициализация структуры для хранения данных о поступающих соединениях
	List* threads_list = NULL;
	listInit(&threads_list, sizeof(ThreadInfo), NULL);

	pid_t serv_pid = getpid();

	// Приём входящих соединений в отдельных потоках
	uint32_t connection_count = 0;
	while (connection_count != UINT_MAX) {
		int client = accept(serv_sock, (struct sockaddr*)&client_addr, &sock_len);
		if (SOCK_ERR < client) {
			printf("Server [%d]: got connection - %u\n", serv_pid, ++connection_count);

			ThreadInfo thread_param = (ThreadInfo){connection_count, client};
			listPushBack(threads_list, (void*)&thread_param);
			
			err = pthread_create(&((ThreadInfo*)listBack(threads_list))->thread, 
								 &thread_attr, thread_func, listBack(threads_list));
			if (err != 0)
				throwErr("Error: cannot create a thread");
		}

		// Очистка данных завершённых потоков, в зависимости от условия
		// Проходим по всем потоками, если существуют, и посылаем сигнал
		if (clearCheck(connection_count, clear_pull) && !listIsEmpty(threads_list)) {
			ListNode* threads_list_iter = threads_list->head;
			do {
				ListNode* check_item = threads_list_iter;
				threads_list_iter = threads_list_iter->next;

				// Проверяем по сигналу, завершил ли поток работу
				// Если завершил -- очищаем все данные связанные с потоком
				if (pthread_kill(((ThreadInfo*)check_item->data)->thread, 0)) {
					pthread_join(((ThreadInfo*)check_item->data)->thread, NULL);
					listDeleteNode(threads_list, check_item);
				}
			} while (threads_list_iter);
		}
	}

	pthread_attr_destroy(&thread_attr);

	listFree(&threads_list);
	close(serv_sock);
}