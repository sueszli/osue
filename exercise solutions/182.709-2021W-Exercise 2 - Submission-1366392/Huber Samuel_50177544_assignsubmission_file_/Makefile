# author:	Huber Samuel 11905181
# programm:	forksort

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = forksort.o

.PHONY: all clean
all: forksort

forksort: $(OBJECTS)
	$(CC) -o forksort forksort.o

forksort.o: forksort.c
	$(CC) $(CFLAGS) -c -o  forksort.o forksort.c

clean:
	rm -rf forksort.o forksort
