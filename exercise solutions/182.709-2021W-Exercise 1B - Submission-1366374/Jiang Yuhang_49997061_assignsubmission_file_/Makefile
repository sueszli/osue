# Author: Yuhang Jiang
# Program name: supervisor generator

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -pthread -lrt -lm

.PHONY: all clean
all: generator supervisor

generator: generator.o circular_buffer.o
	$(CC) -o $@ $^ $(LDFLAGS)

supervisor: supervisor.o circular_buffer.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c circular_buffer.h
generator.o: generator.c circular_buffer.h

clean:
	rm -rf *.o generator supervisor