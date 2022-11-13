# Author: Marvin Flandorfer, 52004069
# Program: cpair

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -pedantic $(DEFS) -g
LDLIBS = -lm

OBJECTS = cpair.o list.o misc.o

.PHONY: all clean
all: cpair

cpair: $(OBJECTS)
	$(CC) -o $@ $^ $(LDLIBS)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

cpair.o: cpair.c list.h misc.h
list.o: list.c list.h misc.h
misc.o: misc.c

clean:
	rm -rf *.o cpair