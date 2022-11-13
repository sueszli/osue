# @file Makefile
# @author Jonas Neumair <e11911064@student.tuwien.ac.at>
# @date 10.12.2021
#
# @brief Makefile for forkFFT.
#
# Builds objects file to execute forkFFT.

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = forkFFT.o

.PHONY: all clean

all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c forkFFT.h

# generates the tgz file with all .c and .h files
tar:
	tar -cvzf Neumair.tgz Makefile *.c *.h

abgabe:
	scp forkFFT.c forkFFT.h constant.txt sine.txt Makefile y11911064@ssh.inflab.tuwien.ac.at:Documents/2

clean:
	rm -rf *.o forkFFT *.tgz