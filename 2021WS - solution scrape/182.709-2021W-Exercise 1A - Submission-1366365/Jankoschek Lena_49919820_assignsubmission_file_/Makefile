# @file Makfile
# @author Lena Jankoschek - 12019852
# @date 2021-11-10
# @brief Makefile for program myexpand
#
# copyright Copyright (c) 2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = myexpand.o

.PHONY = all clean
all: myexpand

myexpand: $(OBJECTS)
		$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $<

myexpand.o: myexpand.c

clean:
		rm -rf *.o myexpand UE1A.tgz

release:
		tar -cvzf UE1A.tgz myexpand.c Makefile
