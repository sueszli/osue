# author:	Huber Samuel 11905181
# programm:	3-coloring

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g
LINKLIB = -lrt -lpthread

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o circular_buffer.o
	$(CC) -o $@ $^ $(LINKLIB)

generator: generator.o circular_buffer.o
	$(CC) -o $@ $^ $(LINKLIB)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c circular_buffer.h
generator.o: generator.c circular_buffer.h
circular_buffer.o: circular_buffer.c circular_buffer.h

clean:
	rm -rf *.o supervisor generator
