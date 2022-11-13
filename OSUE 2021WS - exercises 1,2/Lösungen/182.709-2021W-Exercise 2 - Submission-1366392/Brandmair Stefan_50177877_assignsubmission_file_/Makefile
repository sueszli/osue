# Name: Stefan Brandmair
# Matrikelnummer: 12024754
# Program: forkFFT

LINKERFLAGS = -lm

all: forkFFT

forkFFT: main.o
	gcc -o $@ $^ $(LINKERFLAGS)

%.o: %.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o $@ $<

clean:
	rm -rf *.o forkFFT