DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -g $(DEFINES)
LIBS    = -lpthread

all: server cleanTemp

server: server.o list.o
	gcc $(CFLAGS) -o server server.o list.o $(LIBS)

server.o: server.c list.h
	gcc $(CFLAGS) -c server.c 

list.o: list.c list.h
	gcc $(CFLAGS) -c list.c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf *.o server