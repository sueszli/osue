# @file Makefile
# @author Istvan Kiss <istvan.kiss@student.tuwien.ac.at>
# @date 25.11.2020
#
# @brief Makefile for mycompress program.
CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

OBJECTS = cpair.o

.PHONY: all clean
all: cpair

cpair: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

tar:
	tar -cvzf submission_2.tgz Makefile *.c

clean:
	rm -rf *.o cpair