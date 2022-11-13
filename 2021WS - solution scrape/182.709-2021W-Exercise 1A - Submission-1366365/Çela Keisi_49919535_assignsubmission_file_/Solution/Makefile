# Author: Keisi Cela, 11737582, <e11737582@student.tuwien.ac.at>
# Created: 14.11.2021
# Programs: myexpand

CC      = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS  = -std=c99 -pedantic -Wall -g $(DEFS)

.PHONY: all clean
all: myexpand

myexpand: myexpand.o
	$(CC) -o $@ $^

myexpand.o: myexpand.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o myexpand
