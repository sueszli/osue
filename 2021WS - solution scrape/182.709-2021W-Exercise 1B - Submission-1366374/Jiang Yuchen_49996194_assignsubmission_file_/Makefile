CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o generator.o
LDFLAGS = -lrt -pthread

.PHONY: all clean
all: supervisor generator

supervisor: main.o
	$(CC) -o $@ main.o $(LDFLAGS)

generator: generator.o
	$(CC) -o $@ generator.o $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c main.h

generator.o: generator.c main.h

clean:
	rm -rf *.o supervisor generator