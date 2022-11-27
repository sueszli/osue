# @author Oliver Mayer, MatNr: 12023147
# @date 2021-11-09
# @program ispalindrom

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = main.o isPalindrom.o

.PHONY: all clean
all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

main.o: main.c isPalindrom.h
	$(CC) $(CFLAGS) -c -o $@ $<

isPalindrom.o: isPalindrom.c isPalindrom.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o ispalindrom
