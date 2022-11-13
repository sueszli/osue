CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g -c $(DEFS)
LDFLAGS = -pthread -lrt
# OBJECTS = supervisor.o generator.o

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	$(CC) -o $@ $^ $(LDFLAGS)

generator: generator.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o supervisor generator
