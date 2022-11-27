# author: Márton Károlyi
# program name: forkFFT

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -lm  $(DEFS) -g

OBJECTS = forkFFT.o

.PHONY: all clean
all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c forkFFT.h


clean:
	rm -rf *.o forkFFT