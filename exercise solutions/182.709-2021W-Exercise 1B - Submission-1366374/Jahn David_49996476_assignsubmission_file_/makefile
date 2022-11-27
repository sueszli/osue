#Author: David Jahn, 12020634
#Program name: myGrep

CC = gcc
LDFLAGS = -lrt -pthread
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) 

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o graph.o circular_buffer.o
	$(CC) -o $@ $^ $(LDFLAGS) 

generator: generator.o graph.o circular_buffer.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

supervisor.o: supervisor.c 

clean:
	rm -rf *.o supervisor generator
