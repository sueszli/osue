CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS=-lcrypt

all: listtool

listtool: list.o listtool.o
	$(CC) $(LDFLAGS) -o $@ $^

listtool.o: listtool.c list.h
	$(CC) $(CFLAGS) -c -o $@ $<

test: listtool
	./test.sh

clean:
	rm -f listtool listtool.o
.PHONY: clean
