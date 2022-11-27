# Author: Fisnik Miftari (e11776840)
# Created: 14.11.2021

CC      = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS  = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: mydiff

mydiff: mydiff.o
	$(CC) -o $@ $^

mydiff.o: mydiff.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mydiff