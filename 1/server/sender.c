#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

// Количество аргументов командной строки
#define ARGS_COUNT 2

// Шаблон команды для отправки HTTP запроса
#define COMMAND_TEMPLATE "telnet %s >/dev/null 2>&1"

// Внешняя переменная для приёма сигнала
volatile sig_atomic_t running = true;

// Функция отлова сигнала
static void signalHandler(int sig_num) {
	if (sig_num == SIGINT)
		running = false;
}

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: [host name [port]]\n");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, signalHandler);

	// Формирование команды
	size_t command_len = strlen(COMMAND_TEMPLATE) + strlen(argv[1]) + 1;
	char* command = (char*)malloc(sizeof(char*) * command_len);

	snprintf(command, command_len, COMMAND_TEMPLATE, argv[1]);	

	printf("Sending requests to the address...\n", argv[1]);
	printf("Press \'Ctrl + C\' to exit.");

	// Исполнять без остановки системную команду, пока не будет послан сигнал SIGINT
	// (Нажаты клавиши Ctrl + C)
	while (running) {
		int ret = system(command);
		// Проверка перехвата сигнала системной командой
		if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
			running = false;
	}
	
	free(command);

	return 0;
}
