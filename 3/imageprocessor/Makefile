DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -O2 -g $(DEFINES)
LIBS    = -lm -fopenmp
TARGET  = imageprocessor

all: $(TARGET) cleanTemp

$(TARGET): $(TARGET)_main.o $(TARGET).o
	gcc $(CFLAGS) -o $(TARGET) $(TARGET)_main.o $(TARGET).o $(LIBS)

$(TARGET)_main.o: $(TARGET)_main.c $(TARGET).h
	gcc $(CFLAGS) -c $(TARGET)_main.c $(LIBS)

$(TARGET).o: $(TARGET).c $(TARGET).h
	gcc $(CFLAGS) -c $(TARGET).c $(LIBS)

cleanTemp:
	rm -rf *.o

clean:
	rm -rf $(TARGET)