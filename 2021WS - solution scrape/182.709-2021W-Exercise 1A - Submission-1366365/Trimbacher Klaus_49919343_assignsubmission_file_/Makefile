#@file Makefile of ispalindrom
#Author: Klaus Trimbacher 11908086
#Date: 21.11.2020

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

OBJECTS = ispalindrom.o

.PHONY: all clean
all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	
ispalindrom.o: ispalindrom.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	rm -rf *.o ispalindrom