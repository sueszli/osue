#Author: Jakob Guttmann 11810289
#Programs: generator and supervisor

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

.PHONY: all clean

all: supervisor generator

supervisor: supervisor.o graph.o
	$(CC) -o $@ $^ $(LDFLAGS)

generator: generator.o graph.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

graph.o: graph.c graph.h
generator.o: generator.c circbuf.h graph.h
supervisor.o: generator.c circbuf.h graph.h


clean:
	rm -rf *.o supervisor generator

	