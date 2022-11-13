#author: Nikolaus Peter 11919877 <e11919877@student.tuwien.ac.at>
#date: 31.10.2021
#program name: ispalindrom

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: ispalindrom

ispalindrom:  main.o
	$(CC) $(CFLAGS) -o $@ main.o

main.o:  main.c
	$(CC) $(CFLAGS) -c -o $@ main.c

clean: 
	rm -rf *.o ispalindrom