## ----------------------------------------------------------------------
## author: Maximilian Scharizer
## file: Makefile
## details: creates executable for mycompress.c
## date: 12.11.2021
## ----------------------------------------------------------------------
CC      = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS  = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: mycompress

mydiff: mycompress.o
	$(CC) -o $@ $^

mydiff.o: mycompress.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mycompress