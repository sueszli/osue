CFLAGS=-Wall -g -std=c99 -pedantic -D_XOPEN_SOURCE=500 -D_BSD_SOURCE
LIBS=-lcrypt

.PHONY: clean

all: listtool

listtool: listtool.o
	gcc -o $@ $^ $(LIBS) list.o

listtool.o: listtool.c list.h
	gcc $(CFLAGS) -c -o $@ $< 

clean:
	rm -f listtool listtool.o
