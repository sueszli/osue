

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -g -Wall -pedantic $(DEFS)
CC = gcc

all: myexpand.o
	$(CC) -o myexpand myexpand.o

myexpand.o: myexpand.c
	$(CC) $(CFLAGS) -c -o myexpand.o myexpand.c

clean:
	rm -rf *.o myexpand