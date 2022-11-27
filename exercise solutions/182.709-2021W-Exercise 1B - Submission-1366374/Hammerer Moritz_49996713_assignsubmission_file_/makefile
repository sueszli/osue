# @author: Moritz Hammerer 11902102
# @program name: supervisor, generator

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	$(CC)  -o $@ $^ -lrt -lpthread

generator: generator.o
	$(CC)  -o $@ $^ -lrt -lpthread

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf *.o supervisor generator
