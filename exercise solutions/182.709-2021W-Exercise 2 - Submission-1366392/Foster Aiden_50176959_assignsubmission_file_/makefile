#
# @file makefile
# @author Aiden Foster 11910604
# @date 08.12.2021
#
# @brief used to make forkFFT
#

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -lm -pthread

OBJECTS = forkFFT.o

.PHONY: all clean

all: forkFFT

forkFFT: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c forkFFT.h

clean:
	rm -rf *.o forkFFT
