# @file Makefile
# @author Valentin Schnabl, 11848108
# @date 24.10.2021
# program-names: mycompress.c mycompress.h

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = mycompress.o
.PHONY: all clean
all: mycompress
mycompress: $(OBJECTS)
	$(CC)    $(LDFLAGS)  -o  $@  $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
mycompress.o: mycompress.c mycompress.h
clean:
	rm -rf *.o mycompress