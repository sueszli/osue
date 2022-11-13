# @file makefile
# @author Andreas Himmler 11901924
# @date 9.11.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -pthread -lrt

TARGETS = generator supervisor
GEN_OBJECTS = generator.o 3coloring.o
SUP_OBJECTS = supervisor.o 3coloring.o


.PHONY: all clean

all: $(TARGETS)

generator: $(GEN_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

supervisor: $(SUP_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(GEN_OBJECTS) $(SUP_OBJECTS) $(TARGETS)
