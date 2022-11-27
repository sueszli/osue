CC 	= gcc
DEFS	= -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS	= -std=c99 -pedantic -Wall -g $(DEFS)

all:	forkFFT

forkFFT: forkFFT.o
	$(CC) -o forkFFT forkFFT.o -lm

forkFFT.o: forkFFT.c
	$(CC) $(CFLAGS) -c -o forkFFT.o forkFFT.c -lm

clean:
	rm -rf forkFFT.o forkFFT
