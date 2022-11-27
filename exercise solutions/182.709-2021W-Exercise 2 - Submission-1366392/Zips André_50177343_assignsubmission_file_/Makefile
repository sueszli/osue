# Makefile for forkFFT
# Author: Andre Zips 11811363
.Phony: all clean

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: forkFFT.o
	$(CC) -lm -o forkFFT forkFFT.o

forkFFT.o:
	$(CC) $(CFLAGS) forkFFT.c

clean: 
	rm forkFFT.o
	rm forkFFT