# Written by Firat Berk Cakar, e0828668, for the my compress assigmment
CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o input.o output.o mycompress.o
.PHONY: all clean
all: mycompress

mycompress: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c output.h input.h mycompress.h
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c

input.o: input.c
output.o: output.c
mycompress.o: mycompress.c

clean:
	rm -rf *.o mycompress
