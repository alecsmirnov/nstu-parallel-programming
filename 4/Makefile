DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE -DWITH_MPI
CFLAGS  = -std=c99 -O2 -g $(DEFINES)
LIBS    = -lm
TARGET  = jacobi3d

all: $(TARGET) cleanTemp

$(TARGET): $(TARGET)_main.o $(TARGET).o
	mpicc $(CFLAGS) -o $(TARGET) $(TARGET)_main.o $(TARGET).o $(LIBS)

$(TARGET)_main.o: $(TARGET)_main.c $(TARGET).h
	mpicc $(CFLAGS) -c $(TARGET)_main.c

$(TARGET).o: $(TARGET).c $(TARGET).h
	mpicc $(CFLAGS) -c $(TARGET).c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf $(TARGET)