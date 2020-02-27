DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -O2 -g $(DEFINES)
LIBS    = -lpthread -lm

all: additionalLibCheck server cleanTemp

additionalLibCheck:
	@(test -f list.h || test -f list.c) || ( \
		echo "Missing list data structure!"; \
		echo "Download at: https://github.com/alecsmirnov/doubly-linked-list" "\n"; \
		exit 1; \
	)

server: server_main.o server.o list.o
	gcc $(CFLAGS) -o server server_main.o server.o list.o $(LIBS)

server_main.o: server_main.c server.h
	gcc $(CFLAGS) -c server_main.c

server.o: server.c server.h list.h
	gcc $(CFLAGS) -c server.c 

list.o: list.c list.h
	gcc $(CFLAGS) -c list.c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf *.o server

# list data structure from: https://github.com/alecsmirnov/doubly-linked-list