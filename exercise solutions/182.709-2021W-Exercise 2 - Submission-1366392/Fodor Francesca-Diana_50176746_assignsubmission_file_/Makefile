#@file Makefile
#@author Francesca Diana Fodor, 11808223
#@date 2021-12-11
#@brief compiles and links given files
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LIN = -lm

.PHONY: all clean;
all: forkFFT

forkFFT: forkFFT.o
	gcc $(LIN) -o forkFFT forkFFT.o

forkFFT.o: forkFFT.c
	gcc $(CFLAGS) -c -o forkFFT.o forkFFT.c

clean:
	rm -f *.o forkFFT
