CC = gcc
DEFS: -D_DEFAULT SOURCE -D_BSD_SOURCE - D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS: -std=c99 -pedantic -Wall -g $(DEFS)

all: ispalindrom

ispalindrom: main.o tolowercase.o nospaces.o reverse.o
	$(CC) $(CFLAGS) -o ispalindrom main.o tolowercase.o nospaces.o reverse.o

tolowercase.o: tolowercase.c header.h
	$(CC) $(CFLAGS) -c -o tolowercase.o tolowercase.c

nospaces.o: nospaces.c header.h
	$(CC) $(CFLAGS) -c -o nospaces.o nospaces.c

reverse.o: reverse.c header.h
	$(CC) $(CFLAGS) -c -o reverse.o reverse.c

main.o: main.c header.h
	$(CC) $(CFLAGS) -c -o main.o main.c

clean:
	rm -rf *.o ispalindrom
