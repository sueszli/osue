# @file Makefile
# @author Franziska Ludwig <e1100784@student.tuwien.ac.at>
# @date 11.12.2021

# @brief Makefile for forkFFT

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LFLAGS = -pthread -lrt -lm

OBJECTS = forkFFT.o

.PHONY: all clean
all: forkFFT

forkFFT: forkFFT.o
	$(CC) $(LFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c

clean: 
	rm -rf *.o forkFFT