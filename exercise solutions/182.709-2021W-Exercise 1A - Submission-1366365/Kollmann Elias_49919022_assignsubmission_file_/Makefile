
CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = myexpand.o

.PHONY: all clean

all: myexpand

myexpand:$(OBJECTS)
	$(CC) -o $@ $^

*.o: *.c
	$(CC) $(CFLAGS) -c -o $@ $<

myexpand.o: myexpand.c

clean:
	rm -rf *.o myexpand











