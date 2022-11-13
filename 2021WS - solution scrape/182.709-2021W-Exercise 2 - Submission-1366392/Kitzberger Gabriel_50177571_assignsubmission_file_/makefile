all: forkFFT.o
	gcc forkFFT.o -o forkFFT -lm

forkFFT.o: forkFFT.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c forkFFT.c
	
clean:
	rm -f forkFFT.o forkFFT