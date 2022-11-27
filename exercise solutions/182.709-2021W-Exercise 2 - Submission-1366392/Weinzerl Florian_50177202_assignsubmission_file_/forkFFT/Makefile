all: forkFFT

.PHONY: all clean

forkFFT: forkFFT.o
	gcc -o $@ $^ -lm

%.o: %.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o $@ $<

clean:
	rm forkFFT *.o
