DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -O0 -g $(DEFINES)
LIBS    = -lpthread

all: threadtimestat cleanTemp

threadtimestat: threadtimestat_main.o threadtimestat.o
	gcc $(CFLAGS) -o threadtimestat threadtimestat_main.o threadtimestat.o $(LIBS)

threadtimestat_main.o: threadtimestat_main.c threadtimestat.h
	gcc $(CFLAGS) -c threadtimestat_main.c

threadtimestat.o: threadtimestat.c threadtimestat.h
	gcc $(CFLAGS) -c threadtimestat.c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf threadtimestat