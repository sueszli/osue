# Makefile for program cpair
# Author: MAYERL Thomas, 52004289 <thomas.mayerl@tuwien.ac.at>

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = cpair.o

.PHONY = all clean
all: cpair

cpair: $(OBJECTS)
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cpair.o: cpair.c

clean:
	rm -rf *.o cpair