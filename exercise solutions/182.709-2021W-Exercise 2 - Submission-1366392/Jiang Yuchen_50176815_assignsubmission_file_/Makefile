CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

OBJECTS = forkfft.o

.PHONY: all clean
all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

forkfft.o: forkfft.c

clean:
	rm -rf *.o forkFFT