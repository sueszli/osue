
# @Author: Lukas Jessl 01604985
# @created: 20.11.2021
# @ brief: The idea of this programm is to split the input into two parts
# (odd and even parts), and calculate the fast fourier transformation recursivley

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = forkFFT.o

.PHONY: all clean
all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -lm

	
forkFFT.o: forkFFT.c forkFFT.h


clean:
	rm -rf *.o forkFFT
