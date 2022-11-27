# Author: Hao Hao Ying <e01625773@student.tuwien.ac.at>
# Program name: forkFFT

CC = gcc
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
.PHONY: all clean
all: forkFFT
	

forkFFT: forkFFT.o
	$(CC) -o forkFFT forkFFT.o -lm

forkFFT.o: forkFFT.c
	$(CC) $(FLAGS) forkFFT.c


clean:
	rm -f *.o forkFFT
