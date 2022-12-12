CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)
OBJS=client.o

.PHONY: clean

all: client

client: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

client.o: client.c common.h

clean:
	rm -f client $(OBJS) server-1.log server-2.log
