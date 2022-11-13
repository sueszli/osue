# author:	Huber Samuel 11905181
# programm:	mydiff

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = mydiff.o

.PHONY: all clean
all: mydiff

mydiff: $(OBJECTS)
	$(CC) -o mydiff mydiff.o

mydiff.o: mydiff.c
	$(CC) $(CFLAGS) -c -o  mydiff.o mydiff.c

clean:
	rm -rf mydiff.o mydiff
