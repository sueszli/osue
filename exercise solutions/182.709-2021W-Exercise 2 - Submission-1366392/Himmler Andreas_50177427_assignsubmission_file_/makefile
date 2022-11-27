# @file makefile
# @author Andreas Himmler 11901924
# @date 7.12.2021

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)
LDFLAGS = -lm

TARGETS = forkFFT
OBJECTS = forkFFT.o


.PHONY: all clean

all: $(TARGETS)

forkFFT: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJECTS) $(TARGETS)
