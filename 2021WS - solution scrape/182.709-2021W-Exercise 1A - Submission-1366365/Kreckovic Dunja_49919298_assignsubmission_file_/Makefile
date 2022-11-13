# Author: Dunja Kreckovic (11928274)
# Created: 7.11.2020
# Programs: mygrep

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY = all clean

all: mygrep

mygrep: mygrep.o
	$(CC) $(CFLAGS) -o $@ $<

mygrep.o: mygrep.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf mygrep.o mygrep