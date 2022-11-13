# Programm: supervisor & generator (3-Coloring)
# Author: Christopher Graßl (11816883)
# Date: 14.11.2021


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -lrt -pthread

OBJECTS_GENERATOR = generator.o graphs.o shm.o
OBJECTS_SUPERVISOR = supervisor.o shm.o

.PHONY: all clean

all: supervisor generator

supervisor: $(OBJECTS_SUPERVISOR)
	$(CC) $(LDFLAGS) -o $@ $^

generator: $(OBJECTS_GENERATOR)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o supervisor generator