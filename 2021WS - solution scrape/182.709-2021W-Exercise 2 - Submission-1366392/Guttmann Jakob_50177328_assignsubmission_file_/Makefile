#Author: Jakob Guttmann 11810289
#Programs: cpair

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread -lm

.PHONY: all clean

all: cpair

cpair: main.o points.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

main.o: main.c points.h
points.o: points.c points.h



clean:
	rm -rf *.o cpair

	