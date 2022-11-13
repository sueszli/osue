#Program: mygrep
#Author: Felix Gross 12019865



CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = main.o

.PHONY: all clean
all: mygrep

mygrep: main.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -rf *.o mygrep