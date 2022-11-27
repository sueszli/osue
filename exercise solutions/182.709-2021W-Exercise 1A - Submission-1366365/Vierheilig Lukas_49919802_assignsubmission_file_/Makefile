#\
author: Lukas Vierheilig\
program: ispalindrom \
date: 01/11/2021\

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = 

OBJS = ispalindrom.o

.PHONY: all clean memorytest
all: ispalindrom

ispalindrom: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o *.txt ispalindrom

memorytest:
	make clean && make all && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./ispalindrom.c  

