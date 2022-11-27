# @file Makefile
# @author Istvan Kiss <istvan.kiss@student.tuwien.ac.at>
# @date 08.11.2020
#
# @brief Makefile for supervisor and generator program.

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

LDFLAGS = -lpthread -lrt

SUPERVISOR_OBJECTS = supervisor.o graph.o circbuffer.o shm.o
GENERATOR_OBJECTS = generator.o graph.o circbuffer.o shm.o

.PHONY: all clean
all: generator supervisor

generator: $(GENERATOR_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

supervisor: $(SUPERVISOR_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

generator.o: generator.c graph.h
graph.o: graph.c graph.h

tar:
	tar -cvzf submission_1b.tgz Makefile *.c *.h

clean:
	rm -rf *.o generator supervisor task1b.tgz