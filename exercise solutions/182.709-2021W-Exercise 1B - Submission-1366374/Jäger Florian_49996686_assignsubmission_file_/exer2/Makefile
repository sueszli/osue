#Author: Florian Jäger (11810847)
#Program: fb_arc_set

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
OBJECTS = supervisor.o generator.o

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread -lrt

generator: generator.o
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread -lrt

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -lpthread -lrt

supervisor.o: supervisor.c supervisor.h
generator.o: generator.c generator.h

clean:
	rm -rf *.o supervisor generator