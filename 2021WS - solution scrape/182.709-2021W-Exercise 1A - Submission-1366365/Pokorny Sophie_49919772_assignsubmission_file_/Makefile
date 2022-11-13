# @file Makefile
# @author Sophie Pokorny 01633652 <e01633652@student.tuwien.ac.at>
# @date 12.11.2021
#
# @brief Makefile for myexpand

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = myexpand.o 

.PHONY: all clean
all: myexpand

myexpand: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

myexpand.o: myexpand.c 

clean:
	rm -rf *.o myexpand
