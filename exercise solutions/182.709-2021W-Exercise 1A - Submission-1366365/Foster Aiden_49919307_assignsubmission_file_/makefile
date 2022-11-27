#
# @file makefile
# @author Aiden Foster 11910604
# @date 11.11.2021
#
# @brief used to make mygrep
#

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mygrep.o

.PHONY: all clean

all: mygrep

mygrep: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c mygrep.h

clean:
	rm -rf *.o mygrep
