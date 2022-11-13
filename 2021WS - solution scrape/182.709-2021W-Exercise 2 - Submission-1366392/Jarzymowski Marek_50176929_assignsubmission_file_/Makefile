#@file Makefile
#@author Marek Jarzymowski <e11721384@student.tuwien.ac.at>
CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm
FORKOBJ = forkFFT.o

.PHONY: all clean
all: forkFFT

forkFFT: $(FORKOBJ)
		$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c 
		$(CC) $(CFLAGS) -c -o $@ $<
