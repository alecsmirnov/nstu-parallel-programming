#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

// list data structure from: https://github.com/alecsmirnov/doubly-linked-list
#include "list.h"

#define SERVER_PORT 8080

// Единицы размеров для стека
#define KB 1024
#define MB 1024 * KB

// Размер стека потока по умолчанию
#define DEFAULT_STACK_SIZE   2 * MB
// Размер очереди подключений по умолчанию
#define DEFAULT_CLIENT_COUNT 128

// Указатель на потоковую функцию
typedef void* (*pthread_func)(void*);

// Открытые параметры потока
typedef struct ThreadParam {
	uint32_t request_num;	// Номер запроса (номер потока + 1)
	int client_fd;			// Дескриптор клиента
} ThreadParam;

// Отправить ответ клиенту
void clientWrite(int client_fd, char* response, size_t response_size);
// Закрытие соединения с клиентом
void clientClose(int client_fd);

// Запуск сервера с указанной функцией, размером стека и параметром очистки
// (Параметр очистки: 0 - не чистить, n - чистить через каждые n записей)
void serverStart(pthread_func thread_func, size_t stack_size, size_t clear_pull);

#endif
