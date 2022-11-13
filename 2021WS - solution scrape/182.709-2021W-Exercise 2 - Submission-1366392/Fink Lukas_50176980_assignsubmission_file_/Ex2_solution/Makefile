# author: Lukas Fink, 11911069
# program name: cpair

CC = gcc
DEFS = -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall $(DEFS) -g
LDFLAGS = -lm

.PHONY: all clean
all: cpair

cpair: main.o datatypes.o
	$(CC) -g $(LDFLAGS) -o cpair main.o datatypes.o

main.o: main.c datatypes.h
	$(CC) $(CFLAGS) -c -o main.o main.c

datatypes.o: datatypes.c datatypes.h
	$(CC) $(CFLAGS) -c -o datatypes.o datatypes.c

clean:
	rm -rf *.o cpair
