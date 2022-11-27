#/**
#* @program name forkFFT
#* @author Georges Kalmar 09803393
#**/
CC= gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -lm

.PHONY: all clean

all:
	$(CC) $(CFLAGS) $(LDFLAGS) -o forkFFT forkFFT.c

clean:
	rm forkFFT