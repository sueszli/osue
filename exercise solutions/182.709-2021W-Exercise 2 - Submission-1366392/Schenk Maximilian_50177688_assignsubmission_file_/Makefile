# @file Makefile
# @author Maximilian Schenk 11908528 <e11908528@student.tuwien.ac.at>
# @date 04.12.2021
#
# @brief Makefile for cpair (Exercise 2)

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 
LDFLAGS = -lm # needed for math.h

.PHONY: all clean release

all: cpair

cpair: cpair.o
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o cpair cpair_11908528.tgz


# Create .tgz file
release:
	tar -cvzf cpair_11908528.tgz Makefile *.c