# @file Makefile
# @author Jan Ploner (12025954)
# @date 14.11.2021
# @brief compiles the mygrep.c and builds an executable

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -pedantic -g $(DEFS)

.PHONY: all clean
all: mygrep

mygrep: mygrep.o
	$(CC) $(LDFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c

clean:
	rm -rf *.o mygrep