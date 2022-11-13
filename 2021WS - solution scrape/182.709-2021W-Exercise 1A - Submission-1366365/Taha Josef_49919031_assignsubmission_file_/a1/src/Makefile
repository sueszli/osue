CC = gcc
CFLAGS = -g -std=c99 -pedantic -Wall $(DEFS)
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: all clean

all: mygrep

mygrep: main.o mygrep.o
	gcc $(LDFLAGS) -o $@ $^

main.o: main.c header.h
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c header.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o mygrep


