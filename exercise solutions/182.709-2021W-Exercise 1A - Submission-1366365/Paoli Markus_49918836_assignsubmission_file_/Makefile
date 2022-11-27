# Program: myexpand
# Author: Markus Paoli 01417212

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

PNAME = myexpand
CFILES = main.c util.c
OBJECTS = $(CFILES:.c=.o)

.PHONY: all clean

all: $(PNAME)

$(PNAME): $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c util.h
util.o: util.c util.h

clean:
	rm -f $(PNAME) $(OBJECTS)
