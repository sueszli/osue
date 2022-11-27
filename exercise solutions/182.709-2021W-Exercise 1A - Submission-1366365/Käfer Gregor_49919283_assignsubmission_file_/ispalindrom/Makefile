#makefile for ispalindrom
#@author Gregor Käfer 01326186
#
#@date 13-11-2021
#
#

CC=gcc
DEFS=-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = ispalindrom.o

.PHONY: all clean
all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf ispalindrom
	rm -rf ispalindrom.o