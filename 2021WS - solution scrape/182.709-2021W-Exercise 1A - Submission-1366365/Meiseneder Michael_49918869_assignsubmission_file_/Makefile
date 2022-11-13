#Author: Michael Meiseneder, 51907333, <michael.meiseneder@student.tuwien.ac.at>
#Program Name: myexpand

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
.PHONY: all clean

all: myexpand

myexpand: myexpand.c myexpand.h
	$(CC) $(CFLAGS) $(DEFS) -g -o $@ $<

clean:
	rm -rf myexpand *.o *.tgz

assignment: clean
	tar -cvzf assignment_1A_51907333.tgz ./*
