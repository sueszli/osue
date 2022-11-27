# Autor: kemal Oener
# makefile for mydiff

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTFILES=mydiff.o

.PHONY: all clean
all: mydiff

mydiff: $(OBJECTFILES)
	$(CC) $(LDFLAGS) -o $@ $^

$.o: $.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTFILES) mydiff
