# Makefile for program: 'myexpand'
# @author Laurenz Zehetner 12023972


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS)

.PHONY: all clean

all: myexpand

myexpand:	myexpand.o
	$(CC) $(LDFLAGS) -o $@ $^

myexpand.o:		myexpand.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf *.o myexpand