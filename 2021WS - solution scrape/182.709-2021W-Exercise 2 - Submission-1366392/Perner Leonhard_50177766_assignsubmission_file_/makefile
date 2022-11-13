# @file Makefile
# @author Leonhard Perner <e12020652@student.tuwien.ac.at>
# @date 8.12.2021
#
# @ brief The Makefile for forksort
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = forksort.o

.PHONY: all clean
all: forksort

myexpand: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c

clean:
	rm -rf *.o forksort
