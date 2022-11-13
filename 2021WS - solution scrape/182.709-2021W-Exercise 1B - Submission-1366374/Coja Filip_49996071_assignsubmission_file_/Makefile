# @file Makefile
# @author Filip Coja <e12023141@student.tuwien.ac.at>
# @date 11.11.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread -lpthread -lm

OBJECTS = generator.o graph.o circular_buffer.o shm.o
OBJECTS_supervisor = supervisor.o circular_buffer.o shm.o

.PHONY: all clean
all: generator supervisor clean_after

generator: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

supervisor: $(OBJECTS_supervisor)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

generator.o: generator.c graph.h
supervisor.o: supervisor.c
graph.o: graph.c graph.h
circular_buffer.o: circular_buffer.c circular_buffer.h
shm.o: shm.c shm.h

clean_after:
	rm -rf *.o

clean:
	rm -rf *.o supervisor generator
