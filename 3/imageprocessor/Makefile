DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
LIBS    = -lm -fopenmp
TARGET  = imageprocessor

CF_DEMO = -std=c99 -O2 -g $(DEFINES)
CF_TEST = -std=c99 -O2 -g $(DEFINES) -DTEST

demo: CFLAGS = $(CF_DEMO)
demo: $(TARGET) cleanTemp

test: CFLAGS = $(CF_TEST)
test: $(TARGET) cleanTemp

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