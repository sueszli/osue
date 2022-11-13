.PHONY: all clean

all: isPalindrom.p
	gcc -o isPalindrom isPalindrom.o

isPalindrom.o: isPalindrom.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c isPalindrom.c

clean: /bin/rm -f *.o isPalindrom
