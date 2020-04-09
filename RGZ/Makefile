DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE -DMYMPI
CFLAGS  = -std=c99 -O2 -g $(DEFINES)
LIBS    = -lm
TARGET  = mympi

all: $(TARGET) cleanTemp

mpi: $(TARGET)_main.c
	mpicc $(TARGET)_main.c -o mpi_version

$(TARGET): $(TARGET)_main.o $(TARGET).o
	gcc $(CFLAGS) -o $(TARGET)_version $(TARGET)_main.o $(TARGET).o $(LIBS)

$(TARGET)_main.o: $(TARGET)_main.c $(TARGET).h
	gcc $(CFLAGS) -c $(TARGET)_main.c

$(TARGET).o: $(TARGET).c $(TARGET).h
	gcc $(CFLAGS) -c $(TARGET).c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf $(TARGET) $(TARGET)_version mpi_version