#author Andreas Huber 11809629
#program fib_arc_set including circularBuffer.c, circularBuffer.h, generator.c, supervisor.c

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

.PHONY: all clean
all: supervisor generator

generator: generator.o circularBuffer.o
	$(CC) $(LDFLAGS) -o $@ $^

supervisor: supervisor.o circularBuffer.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o generator supervisor
	

