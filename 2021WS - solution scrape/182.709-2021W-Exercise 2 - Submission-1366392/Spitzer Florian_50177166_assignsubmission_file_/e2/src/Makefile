# Project name: forkFFT
# Author: Florian Spitzer, Student ID 00507913

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lm

all: forkFFT

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

forkFFT.o: forkFFT.c

forkFFT: forkFFT.o
	$(CC) -o forkFFT forkFFT.o $(LDFLAGS)

clean:
	rm forkFFT *.o
