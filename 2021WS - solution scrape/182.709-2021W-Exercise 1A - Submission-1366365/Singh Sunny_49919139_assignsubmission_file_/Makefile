CFLAG= -Wall -g -std=c99 -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: clean ispalindrom

ispalindrom: ispalindrom.o
	@gcc $(CFLAG) -o bin/ispalindrom bin/ispalindrom.o

ispalindrom.o: ispalindrom.c
	@mkdir -p bin
	@gcc -c ispalindrom.c -o bin/ispalindrom.o

clean:
	@rm -rf bin/*

abgabe: all
	@mkdir -p abgabe
	@rm -rf abgabe/*
	@cp --no-preserve=all *.c abgabe/
	@cp --no-preserve=all Makefile abgabe/
	@cd abgabe && tar -cvzf 00271187.tgz *
	echo "abgabe fertig!"