# Author: Marvin Flandorfer, 52004069
# Program: mycompress

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -pedantic $(DEFS) -g

OBJECTS = mycompress.o filein.o fileout.o misc.o

.PHONY: all clean
all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c filein.h fileout.h misc.h
filein.o: filein.c misc.h
fileout.o: fileout.c misc.h
misc.o: misc.c misc.h

clean:
	rm -rf *.o mycompress