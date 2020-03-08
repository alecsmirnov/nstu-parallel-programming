#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "server.h"

#define ARGS_COUNT 4

// Параметры ожидания для ждущего потока
#define TIME_MINUTE     60		// Минута
#define THREAD_WAIT_MIN 10 		// Кол-во минут

// Препроцессор для формирования строковых констант
#define TEXT_QUOTE(...) #__VA_ARGS__

// Количество цифр десятичного числа
#define intDigitsCount(val)	\
	((val) == 0 ? 1 : (size_t)floor(log10(abs(val))) + 1)

// Шаблон для стандартной функции
static const char* RESPONSE_TEMPLATE = TEXT_QUOTE(
	HTTP/1.1 200 OK\r\n
	Content-Length: %lu\r\n\r\n
		<html>
			<head>
				<title>Request</title>
				<style>
					h1 {
						text-align: center;
					}
				</style>
			</head>
		<body>
			<h1>Request number %u has been processed</h1>
		</body>
	</html>\r\n
);

// Шаблон для возврата в версии PHP
static const char* RESPONSE_TEMPLATE_PHP = TEXT_QUOTE(
	HTTP/1.1 200 OK\r\n
	Content-Length: %lu\r\n\r\n
		<html>
			<head>
				<title>Request</title>
				<style>
					h1, div {
						text-align: center;
					}
				</style>
			</head>
		<body>
			<h1>Request number %u has been processed</h1>
			<div>PHP version: %s</div>
		</body>
	</html>\r\n
);

// Стандартная потоковая функция
static void* threadFunc(void* arg) {
	ThreadParam* thread_param = (ThreadParam*)arg;

	size_t response_size = strlen(RESPONSE_TEMPLATE) + 
						   intDigitsCount(thread_param->request_num);
	char* response = (char*)malloc(sizeof(char) * response_size);
	snprintf(response, response_size, RESPONSE_TEMPLATE, response_size, 
			 thread_param->request_num);

	clientWrite(thread_param->client_fd, response, response_size);
	clientClose(thread_param->client_fd);

	free(response);
	pthread_exit(NULL);
}

// Потоковая функция с вызовом интерпретатора PHP и возвратом версии PHP
static void* threadFuncPHP(void* arg) {
	ThreadParam* thread_param = (ThreadParam*)arg;

	enum {PHP_VERSION_LEN = 20};
	char php_version[PHP_VERSION_LEN];

	FILE* fp = popen("php -r \"echo phpversion();\"", "r");
	if (fscanf(fp, "%s", php_version) == 1) {
		pclose(fp);

		size_t response_size = strlen(RESPONSE_TEMPLATE_PHP) + 
							   intDigitsCount(thread_param->request_num) + PHP_VERSION_LEN;
		char* response = (char*)malloc(sizeof(char) * response_size);
		snprintf(response, response_size, RESPONSE_TEMPLATE_PHP, 
				 response_size, thread_param->request_num, php_version);
		
		clientWrite(thread_param->client_fd, response, response_size);
		free(response);
	}

	clientClose(thread_param->client_fd);
	pthread_exit(NULL);
}

// Потоковая функция не прекращающая работу
static void* threadFuncWait(void* arg) {
	ThreadParam* thread_param = (ThreadParam*)arg;
	clientClose(thread_param->client_fd);

	sleep(THREAD_WAIT_MIN * TIME_MINUTE);

	pthread_exit(NULL);	
}

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <func num> <stack size num> <clear pull>\n");
		fprintf(stderr, "(Func number: 0 - std, 1 - php, 2 - wait; "
						"Stack size num: 0 - 512 KB, 1 - 1 MB, 2 - 2 MB)\n");
		exit(EXIT_FAILURE);
	}

	uint8_t func_num = atoi(argv[1]);
	uint8_t stack_size_num = atoi(argv[2]);
	size_t clear_pull = atol(argv[3]);

	pthread_func thread_func = threadFunc;
	switch (func_num) {
		case 0: thread_func = threadFunc;     break;
		case 1: thread_func = threadFuncPHP;  break;
		case 2: thread_func = threadFuncWait; break;
	}

	size_t stack_size = DEFAULT_STACK_SIZE;
	switch (stack_size_num) {
		case 0: stack_size = 512 * KB; break;
		case 1: stack_size =   1 * MB; break;
		case 2: stack_size =   2 * MB; break;
	}

	serverStart(thread_func, stack_size, clear_pull);
	
	return 0;
}
