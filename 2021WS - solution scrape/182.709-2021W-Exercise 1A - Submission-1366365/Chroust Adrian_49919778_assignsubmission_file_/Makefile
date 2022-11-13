# Author: Adrian Chroust <e12023146@student.tuwien.ac.at>
# Date: 27.10.2021

# Name of the executable binary file
MODULE = ispalindrom

# Compiler flags
CC = gcc
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
COMPILE = $(CC) $(FLAGS)

.PHONY: all clean

all: $(MODULE)

clean:
	rm -rf src/*.o $(MODULE)

$(MODULE): src/main.o src/palindrome.o
	$(COMPILE) -o $@ $^

%.o: %.c
	$(COMPILE) -c -o $@ $<

src/main.o: src/main.c src/palindrome.h
src/palindrome.o: src/palindrome.c src/palindrome.h
