#author: Jasmine Haider 11943664
#exercise 2 - Fork Fourier Transformation

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -c -std=c99 -pedantic $(DEFS)

OBJECTS = forkFFT.o

.PHONY: all clean
all: forkFFT

forkFFT: forkFFT.o
	gcc $(LDFLAGS) -lm -o $@ $^

%.o: %.c 
	gcc $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c forkFFT.h

clean:
	rm -rf *.o forkFFT
