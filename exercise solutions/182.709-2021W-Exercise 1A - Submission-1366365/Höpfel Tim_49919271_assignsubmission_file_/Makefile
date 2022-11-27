CC = gcc #C compiler
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mycompress.o compresshelper.o

.PHONY: all clean
#PHONY executed without checking for existing files

all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^		#linking

%.o:  %.c
	$(CC) $(CFLAGS) -c -o $@ $<		#compiling

mycompress.o: mycompress.c compresshelper.h
compresshelper.o: compresshelper.c compresshelper.h

clean:
	rm -rf *o mycompress
#use clean before uploading!
