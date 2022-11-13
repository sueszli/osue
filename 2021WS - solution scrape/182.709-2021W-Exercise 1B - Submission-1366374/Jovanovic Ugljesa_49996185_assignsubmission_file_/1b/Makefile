CC=gcc
CCFLAGS=-Wall -std=c99 -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_SOURCE=200809L -g
LDFLAGS= -pthread -lrt

SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

all: supervisor generator

supervisor: supervisor.o circ_buf.o
	$(CC) -o $@ $^ $(LDFLAGS)

generator: generator.o circ_buf.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)
	rm -f supervisor generator
