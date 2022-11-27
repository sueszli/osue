# @file ispalindrom
# @author Nico Lehegzek 12021356
# @date 05.11.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_DEFAULT_SOURCE -D_POSIC_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = ispalindrom.o

.PHONY: all clean
all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

ispalindrom.o: ispalindrom.c

clean:
	rm -rf *.o ispalindrom