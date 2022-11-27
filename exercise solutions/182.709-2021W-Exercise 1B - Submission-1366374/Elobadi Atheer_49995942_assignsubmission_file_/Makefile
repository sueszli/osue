# Author: Atheer Elobad 01049225
# Programs: supervisor and generator
# Date: 08.11.2021

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -std=c99 -pedantic -Wall -g -c $(DEFS)
LDFLAGSS = -lrt -pthread


.PHONY: all clean
objects = supervisor generator
all:  $(objects)


supervisor: supervisor.o
	$(CC) $(LDFLAGSS) -o $@ $^

generator: generator.o generatorfunctions.o
	$(CC) $(LDFLAGSS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -lrt

clean:
	rm -rf *.o generator supervisor
