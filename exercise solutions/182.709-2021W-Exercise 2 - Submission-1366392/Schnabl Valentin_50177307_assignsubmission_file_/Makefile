# @file Makefile
# @author Valentin Schnabl, 11848108
# @date 23.11.2021
# program-name: cpair.c, cpair.h

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lrt -pthread -lm
OBJECTS = cpair.o
.PHONY: all clean
all: cpair
cpair: $(OBJECTS)
	$(CC)    $(LDFLAGS)  -o  $@  $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
cpair.o: cpair.c cpair.h
clean:
	rm -rf *.o cpair