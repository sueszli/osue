#@author Dominik Stepan(01327601)
#@file Makefile

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS  = -std=c99 -pedantic -Wall  -g $(DEFS)

OBJECTS = main.o

.PHONY: all clean
	all: main

all: mydiff

mydiff: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mydiff