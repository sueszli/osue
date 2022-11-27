#Name: @author Ruben Patscheider
#MatrikelNummer: 01627951
#Program: forksort

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
POST = -lm

.PHONY: all clean

all: forksort

forksort: forksort.o functions.o
	$(CC) $(CFLAGS) -o $@ $^ $(POST)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *.o forksort
