# Author: Mathias Möler
# Program Name: forkFFT

all: forkFFT

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -lm

forkFFT: forkFFT.o
	$(CC) $(CFLAGS) -o forkFFT forkFFT.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c forkFFT.h

clean:
	rm -rf forkFFT forkFFT.o