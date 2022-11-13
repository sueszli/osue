# @file Makefile
# @author Filip Coja <e12023141@student.tuwien.ac.at>
# @date 05.12.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: forksort clean_after

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort: forksort.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean_after:
	rm -rf *.o

clean:
	rm -rf *.o forksort
