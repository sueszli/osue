#author: Marcin Boniecki
#program name: Feedback Arc Set Generator
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread -lrt

generator: generator.o
	$(CC) $(LDFLAGS) -o $@ $^ -pthread -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c supervisor.h
generator.o: generator.c generator.h

clean:
	rm -rf *.o supervisor generator
 