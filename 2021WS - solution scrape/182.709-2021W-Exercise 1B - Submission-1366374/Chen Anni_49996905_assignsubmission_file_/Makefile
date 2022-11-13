# Author: Anni Chen (11902050)
# Created: 3.11.2021
# Programs: generator, supervisor

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) -fdiagnostics-color=always
LDFLAGS = -lrt -pthread

GENERATOR_OBJECTS = generator.o 3color.o circular_buffer.o
SUPERVISOR_OBJECTS = supervisor.o 3color.o circular_buffer.o

.PHONY: all clean
all: generator supervisor

generator: $(GENERATOR_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

supervisor: $(SUPERVISOR_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o generator supervisor