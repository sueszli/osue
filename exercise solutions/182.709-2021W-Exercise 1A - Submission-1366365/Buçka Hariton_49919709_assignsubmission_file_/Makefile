# /**
# * @name Makefile
# * @author Hariton Bucka (01529018)
# */

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -std=c99 -pedantic $(DEFS)

OBJECTS = mycompress.o

.PHONY: all clean
all: mycompress

%.o: %.c
	$(CC) $(CFLAGS) -g -c -o $@ $<

mycompress.o: mycompress.c

clean:
	rm -rf *.o mycompress