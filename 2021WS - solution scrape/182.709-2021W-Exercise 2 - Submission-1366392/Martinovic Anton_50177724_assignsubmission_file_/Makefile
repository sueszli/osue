CFLAGS = --std=c99 --pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean

all: forkFFT

forkFFT: forkFFT.o
	gcc -o forkFFT forkFFT.o -lm

forkFFT.o: forkFFT.c
	gcc $(CFLAGS) -c -g -o forkFFT.o forkFFT.c 

clean:
	rm -rf forkFFT.o forkFFT
