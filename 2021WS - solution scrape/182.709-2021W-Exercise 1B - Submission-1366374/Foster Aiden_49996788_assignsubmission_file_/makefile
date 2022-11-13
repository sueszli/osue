#
# @file makefile
# @author Aiden Foster 11910604
# @date 12.11.2021
#
# @brief used to make minimalSet3Coloring
#

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread

OBJECTS_GENERATOR = minimalSet3Coloring_generator.o
OBJECTS_SUPERVISOR = minimalSet3Coloring_supervisor.o

.PHONY: all clean

all: minimalSet3Coloring_generator minimalSet3Coloring_supervisor

minimalSet3Coloring_supervisor: $(OBJECTS_SUPERVISOR)
	$(CC) $(LDFLAGS) -o $@ $^

minimalSet3Coloring_generator: $(OBJECTS_GENERATOR)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

minimalSet3Coloring_generator.o: minimalSet3Coloring_generator.c minimalSet3Coloring_generator.h minimalSet3Coloring.h
minimalSet3Coloring_supervisor.o: minimalSet3Coloring_supervisor.c minimalSet3Coloring_supervisor.h minimalSet3Coloring.h

clean:
	rm -rf *.o minimalSet3Coloring_generator minimalSet3Coloring_supervisor
