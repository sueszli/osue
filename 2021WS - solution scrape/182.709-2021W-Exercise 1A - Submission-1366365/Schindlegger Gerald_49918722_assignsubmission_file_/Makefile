#Name: Gerald Schindlegger
#MtrNr.: 01255532
#Program: myexpand
#Exercise: 1A

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTFILES = myexpand.o


.PHONY: all clean

all: myexpand

myexpand: $(OBJECTFILES)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

myexpand.o: myexpand.c

clean:
	rm -f $(OBJECTFILES) myexpand
