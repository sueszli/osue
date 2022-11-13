# Assignment A - mycompress
# Author: Matthias Grausgruber <e00525708@student.tuwien.ac.at>

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g -c $(DEFS)
OBJECTS = mycompress.c

.PHONY: all clean

all: mycompress

mycompress: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o mycompress
