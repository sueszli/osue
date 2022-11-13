# @file Makefile
# @author Philipp Holzer <e12028208@student.tuwien.ac.at>
# @date 14.11.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mycompress.o

.PHONY: all clean tar
all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mycompress.o: mycompress.c mycompress.h

# generates tgz file with all .c and .h files
tar:
	tar -cvzf Exercise1A.tgz Makefile *.c *.h
	
clean:
	rm -rf *.o mycompress Exercise1A.tgz