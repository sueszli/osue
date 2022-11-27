# @file Makefile
# @author Philipp Slowak (01427655) <e1427655@student.tuwien.ac.at>
# @date 25.10.2021
#
# @brief Makefile for mygrep

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = mygrep.o

.PHONY: all clean
all: mygrep 

mygrep: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
 
mygrep.o: mygrep.c

clean:
	rm -rf *.o mygrep
