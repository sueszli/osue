CC = gcc
CFLAGS = -std=c99 -pedantic
DEFS = -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
FULLCOMPILE = $(CC) $(CFLAGS) $(DEFS)

.PHONY: all clean

all: mycompress_td

mycompress_td: mycompress_td.o
	$(CC) -o $@ $<

mycompress_td.o: mycompress_td.c
	$(FULLCOMPILE) $<

clean:
	rm -rf *.o mycompress_td
