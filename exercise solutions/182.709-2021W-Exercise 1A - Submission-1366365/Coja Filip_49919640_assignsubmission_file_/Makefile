# @file Makefile
# @author Filip Coja <e12023141@student.tuwien.ac.at>
# @date 07.11.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: mydiff clean_after

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mydiff: mydiff.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean_after:
	rm -rf *.o

clean:
	rm -rf *.o mydiff
