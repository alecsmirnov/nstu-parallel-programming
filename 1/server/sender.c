#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#define ARGS_COUNT 2

#define COMMAND_TEMPLATE "telnet %s >/dev/null 2>&1"

volatile sig_atomic_t running = true;

static void signalHandler(int sig_num) {
	if (sig_num == SIGINT)
		running = false;
}

int main(int argc, char* argv[]) {
	if (argc < ARGS_COUNT) {
		fprintf(stderr, "Enter: [host name [port]]\n");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, signalHandler);

	size_t command_len = strlen(COMMAND_TEMPLATE) + strlen(argv[1]) + 1;
	char* command = (char*)malloc(sizeof(char*) * command_len);

	snprintf(command, command_len, COMMAND_TEMPLATE, argv[1]);	

	size_t send_count = 0;
	while (running) {
		printf("Request %zu to address: %s (Ctrl + C -- EXIT)\n", send_count++, argv[1]);
		
		int ret = system(command);
		if (WIFSIGNALED(ret) && (WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
			running = false;
	}
	
	free(command);

	return 0;
}