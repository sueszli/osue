#Author: David Jahn, 12020634
#Program name: forkFFT

CC = gcc
LDFLAGS = -lm
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS) 
OBJECTS = main.o 

.PHONY: all clean
all: forkFFT 

forkFFT: $(OBJECTS) 
	$(CC) -o $@ $^ $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c 

clean:
	rm -rf *.o forkFFT
