#author: Nikolaus Peter 11919877 <e11919877@student.tuwien.ac.at>
#date: 10.12.2021
#program name: intmul

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: intmul

intmul:  intmul.o
	$(CC) $(CFLAGS) -o $@ intmul.o

intmul.o:  intmul.c
	$(CC) $(CFLAGS) -c -o $@ intmul.c

clean: 
	rm -rf *.o intmul