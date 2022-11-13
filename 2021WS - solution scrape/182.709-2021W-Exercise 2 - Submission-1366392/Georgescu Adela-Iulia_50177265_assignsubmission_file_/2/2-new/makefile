# author: Adela-Iulia Georgescu, 11810894
# programname: forkfft

.PHONY: clean

all: forkfft

forkfft: forkfft.o
	gcc -o forkfft forkfft.o -lm

forkfft.o: forkfft.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c forkfft.c

clean:
	rm forkfft forkfft.o