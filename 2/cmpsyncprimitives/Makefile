DEFINES = -D_POSIX_C_SOURCE -D_BSD_SOURCE
CFLAGS  = -std=c99 -O2 -g $(DEFINES)
LIBS    = -lpthread -lm
TARGET  = cmpsyncprimitives

all: $(TARGET) additionalLibCheck cleanTemp

additionalLibCheck:
	@(test -f list.h || test -f list.c) || ( 										\
		echo "Missing list data structure!"; 										\
		echo "Download at: https://github.com/alecsmirnov/doubly-linked-list" "\n"; \
		exit 1; 																	\
	)

$(TARGET): $(TARGET)_main.o $(TARGET).o list.o
	gcc $(CFLAGS) -o $(TARGET) $(TARGET)_main.o $(TARGET).o list.o $(LIBS)

$(TARGET)_main.o: $(TARGET)_main.c $(TARGET).h
	gcc $(CFLAGS) -c $(TARGET)_main.c

$(TARGET).o: $(TARGET).c $(TARGET).h list.h
	gcc $(CFLAGS) -c $(TARGET).c

list.o: list.c list.h
	gcc $(CFLAGS) -c list.c

cleanTemp:
	rm -rf *.o

clean:
	rm -rf $(TARGET)