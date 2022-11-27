CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

CC = gcc
OBJECTS = forkFFT.o

.PHONY: all clean

all : forkFFT

forkFFT : $(OBJECTS)
	$(CC) -o $@ $^ -lm

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o : forkFFT.c forkFFT.h

clean :
	rm -rf *.o forkFFT

