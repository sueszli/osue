#Author: Stefan Theuerer
#Name: mygrep

CC=gcc
DEFS=-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS=-Wall -g -c -std=c99 -pedantic -Wall $(DEFS)

OBJECTS= mygrep.o

.PHONY: all clean

all: mygrep

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) mygrep
