# author: Daniel Reinbacher (01614435)
# programe name: forkFFT

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean
all: forkFFT

forkFFT: forkFFT.o errorhandler.o
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c errorhandler.h
errorhandler.o: errorhandler.c errorhandler.h

clean:
	rm -rf *.o forkFFT