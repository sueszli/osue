CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o searcher.o io_handler.o

.PHONY: all clean debug

all: mygrep


mygrep: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c searcher.h
searcher.o: searcher.c searcher.h

clean:
	rm -rf *.o mygrep

