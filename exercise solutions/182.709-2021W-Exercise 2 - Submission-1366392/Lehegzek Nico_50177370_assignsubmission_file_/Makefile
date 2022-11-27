# @file intmul.c
# @author Nico Lehegzek 12021356
# @date 05.12.2021

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm
#-lm needed for math.h

OBJECTS = intmul.o

.PHONY: all clean
all: intmul

intmul: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

intmul.o: intmul.c

clean:
	rm -rf *.o intmul