DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -O2 -g $(DEFINES)
LIBS    = -lpthread -lm

all: arrayprocessing cleanTemp

arrayprocessing: arrayprocessing_main.o arrayprocessing.o
	gcc $(CFLAGS) -o arrayprocessing arrayprocessing_main.o arrayprocessing.o $(LIBS)

arrayprocessing_main.o: arrayprocessing_main.c arrayprocessing.h
	gcc $(CFLAGS) -c arrayprocessing_main.c

arrayprocessing.o: arrayprocessing.c arrayprocessing.h
	gcc $(CFLAGS) -c arrayprocessing.c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf arrayprocessing