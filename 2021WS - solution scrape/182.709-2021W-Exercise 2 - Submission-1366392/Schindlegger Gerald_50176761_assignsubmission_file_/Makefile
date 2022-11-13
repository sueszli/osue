#Name: Gerald Schindlegger
#MtrNr.: 01255532
#Program: forksort
#Exercise: 2

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTFILES = forksort.o


.PHONY: all clean

all: forksort

myexpand: $(OBJECTFILES)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c

clean:
	rm -f $(OBJECTFILES) forksort
