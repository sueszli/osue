# NAME: AGATA POGORSKA
# 12023151
# PROGRAMNAME: mydiff

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: mydiff

mydiff: mydiff.o
	$(CC) -o $@ $^

mydiff.o : mydiff.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mydiff
