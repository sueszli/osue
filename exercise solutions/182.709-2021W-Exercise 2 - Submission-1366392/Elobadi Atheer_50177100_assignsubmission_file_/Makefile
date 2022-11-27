# Author: Atheer Elobad 01049225
# Programs: forkFFT
# Date: 08.11.2021

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS = -std=c99 -pedantic -Wall -g -c $(DEFS)
LDFLAGSS = -lrt -pthread -lm


.PHONY: all clean
objects = forkFFT
all:  $(objects)


forkFFT: forkFFT.o forkFFT_Funcs.o
	$(CC) $(LDFLAGSS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -lrt

clean:
	rm -rf *.o forkFFT
