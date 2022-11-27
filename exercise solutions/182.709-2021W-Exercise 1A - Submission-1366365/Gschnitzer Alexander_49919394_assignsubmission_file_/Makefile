# @file Makefile
# @author Alexander Gschnitzer (01652750) <e1652750@student.tuwien.ac.at>
# @date 19.10.2021
#
# @brief Makefile for mydiff program.

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g  $(DEFS)
OBJECTS = mydiff.o
TARGETS = mydiff
COMPRESS = mydiff.tgz

.PHONY: all clean build
all: mydiff

mydiff: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mydiff.o: mydiff.c

clean:
	rm -rf *.o $(TARGETS) $(COMPRESS)

build:
	tar -cvzf $(COMPRESS) mydiff.c Makefile

