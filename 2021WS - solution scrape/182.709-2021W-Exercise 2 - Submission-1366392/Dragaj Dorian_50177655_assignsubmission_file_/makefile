# @file Makefile
# @author Dorian Dragaj 11702371
# @date 03.12.2021
#
# @ brief The Makefile for forksort

CC      = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS  = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: forksort

forksort: forksort.o
	$(CC) -o $@ $^

forksort.o: forksort.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o forksort

tar:
	tar -cvzf archive-Task2.tgz makefile forksort.c