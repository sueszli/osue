# Author: Dunja Kreckovic (11928274)
# Created: 8.12.2021
# Programs: forkFFT

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -lm

.PHONY: all clean
all: forkFFT

forkFFT: forkFFT.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -rf forkFFT

