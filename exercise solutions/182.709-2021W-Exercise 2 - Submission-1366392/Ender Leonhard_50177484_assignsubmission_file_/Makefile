# Makefile for exercise 2
# author Leonhard Ender (12027408)
# date 10.12.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = cpair.o
LDLIBS = -lm

all: cpair

cpair: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm cpair *.o
