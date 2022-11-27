# OPERATING SYSTEMS EXERCISE 2
# Assignment - Closest Pair of Points
# Author: Matthias Grausgruber <e00525708@student.tuwien.ac.at>
# Date: 2021-11-26

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L #-DDEBUG
CFLAGS = -std=c99 -pedantic -Wall -g -c $(DEFS)
LDFLAGS = -lm
OBJECTS = cpair.o

.PHONY: all clean
all: cpair

cpair: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o cpair
