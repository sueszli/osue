# author: René Schmidt - 11809925
# program name: mygrep.c
# date: 03.11.2021

.PHONY: clean all

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -g -Wall $(DEFS)

OBJECTFILES = mygrep.o

all: mygrep

mygrep: $(OBJECTFILES)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c

clean:
	rm -f $(OBJECTFILES) mygrep
