CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D -D_POSIX_C_SOURCE=200809L
CFLAFS = -Wall -g std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: mycompress

mycompress: mycompress.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean: 
	rm -rf *.o mycompress
 
