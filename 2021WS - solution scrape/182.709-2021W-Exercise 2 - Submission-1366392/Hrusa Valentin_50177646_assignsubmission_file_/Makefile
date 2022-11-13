#@Author: Valentin Hrusa 11808205
.PHONY: clean all

all: forkFFT

forkFFT: forkFFT.o
	gcc -o forkFFT forkFFT.o -lm

forkFFT.o: forkFFT.c forkFFT.h
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c forkFFT.c

clean:
	rm forkFFT.o forkFFT