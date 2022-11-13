CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS_SUPERVISOR = supervisor.o util.o
OBJECTS_GENERATOR = generator.o util.o

.PHONY: all clean
all: supervisor generator

supervisor: $(OBJECTS_SUPERVISOR)
	$(CC) $(LDFLAGS) -o $@ $^ -pthread
	
generator: $(OBJECTS_GENERATOR)
	$(CC) $(LDFLAGS) -o $@ $^ -pthread

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c util.h
generator.o: generator.c util.h
util.o: util.c util.h

clean:
	rm -rf *.o supervisor generator
