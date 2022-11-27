CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LINKING = -lrt -pthread

OBJECTS = ispalindrom.o
.PHONY: all clean

all: generator supervisor

generator: generator.o circularbuffer.o graph.o
	$(CC) $(DEFS) -o $@ $^ $(LINKING)

supervisor: supervisor.o circularbuffer.o graph.o
	$(CC) $(DEFS) -o $@ $^ $(LINKING)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c shm.h circularbuffer.h

generator.o: generator.c shm.h graph.h circularbuffer.h

circularbuffer.o: circularbuffer.c circularbuffer.h graph.h

graph.o: graph.c graph.h

clean:
	rm -rf *.o main