# Author: Alyssa Scheer 11909736
# Program name: ispalindrom.c

CFLAGS := -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: output

output: ispalindrom.o
	gcc $(CFLAGS) ispalindrom.o -o output

ispalindrom.o: ispalindrom.c
	gcc $(CFLAGS) -c ispalindrom.c

clean:
	rm -f output ispalindrom.o