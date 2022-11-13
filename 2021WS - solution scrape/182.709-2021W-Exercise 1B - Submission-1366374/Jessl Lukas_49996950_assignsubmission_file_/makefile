
CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = generator.o
OBJECTS2 = supervisor.o

.PHONY: all clean
all: supervisor generator

generator: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -lpthread -lrt

supervisor: $(OBJECTS2)
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -lpthread -lrt
	
supervisor.o: supervisor.c supervisor.h
generator.o : generator.c generator.h


clean:
	rm -rf *.o supervisor generator
