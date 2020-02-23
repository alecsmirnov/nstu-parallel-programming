DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -g $(DEFINES)
LIBS    = -lpthread -lm

all: server sender cleanTemp

server: server_main.o server.o list.o
	gcc $(CFLAGS) -o server server_main.o server.o list.o $(LIBS)

server_main.o: server_main.c server.h
	gcc $(CFLAGS) -c server_main.c

server.o: server.c server.h list.h
	gcc $(CFLAGS) -c server.c 

list.o: list.c list.h
	gcc $(CFLAGS) -c list.c

sender: sender.c
	gcc $(CFLAGS) -o sender sender.c 

cleanTemp:
	rm -rf *.o

clean:
	rm -rf *.o server