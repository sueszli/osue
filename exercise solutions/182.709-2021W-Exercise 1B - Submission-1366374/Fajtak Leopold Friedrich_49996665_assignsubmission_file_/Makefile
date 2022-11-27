# @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
# @programNames supervisor generator

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCHE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g
LFLAGS = $(CFLAGS)

.PHONY: all clean
all: supervisor generator

supervisor: supervisor.o circularbuffer.o graph.o
	$(CC) $(LFLAGS) -o $@ $^ -lrt -pthread
generator: generator.o circularbuffer.o graph.o
	$(CC) $(LFLAGS) -o $@ $^ -lrt -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o supervisor generator
