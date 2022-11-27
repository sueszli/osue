# author: Lukas Fink, 11911069
# program name: mycompress

CC = gcc
DEFS = -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY: all clean
all: mycompress

mycompress: main.o comp.o
	$(CC) $(LDFLAGS) -o mycompress main.o comp.o

main.o: main.c comp.h
	gcc $(CFLAGS) -c -o main.o main.c

comp.o: comp.c comp.h
	gcc $(CFLAGS) -c -o comp.o comp.c

clean:
	rm -rf *.o mycompress