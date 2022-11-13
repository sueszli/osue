# author: Gernot Hahn 01304618
# program name: fb_arc_set

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -pthread -lrt $(DEFS)

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o graph.o util.o
	$(CC) -o $@ $^ $(LDFLAGS)

generator: generator.o graph.o util.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c graph.h util.h
generator.o: generator.c graph.h util.h
graph.o: graph.c graph.h
util.o: util.c util.h

clean:
	rm -rf *.o supervisor generator

