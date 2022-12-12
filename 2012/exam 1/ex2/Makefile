CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
OBJS=client.o

all: client

client: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test: client
	./test.sh

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f client $(OBJS) server-1.log server-2.log
.PHONY: clean

client.o: client.c common.h
