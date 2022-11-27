# Author: Ruben Patscheider
# MatrikelNummer: 01627951
# Program: mydiff


CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: mydiff

mydiff: mydiff.o
	$(CC) $(CFLAGS) -o mydiff mydiff.o

mydiff.o: mydiff.c
	$(CC) $(CFLAGS) -c -o mydiff.o mydiff.c

clean:
	rm -rf *.o mydiff
