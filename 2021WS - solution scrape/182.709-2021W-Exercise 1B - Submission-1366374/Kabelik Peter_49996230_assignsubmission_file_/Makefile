# Author: Peter Kabelik (01125096)
# Program names: supervisor, generator (Feedback Arc Set)

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

all: supervisor generator

supervisor: supervisor.o common.o
	$(CC) -o $@ $^ $(LDFLAGS)
	
generator: generator.o common.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c common.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o supervisor generator
	
.PHONY: all clean
