#Author: Stefan Theuerer
#Name: forkFFT

CC=gcc
DEFS=-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS=-Wall -g -c -std=c99 -pedantic -Wall $(DEFS)

OBJECTS= forkFFT.o

.PHONY: all clean

all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) forkFFT
