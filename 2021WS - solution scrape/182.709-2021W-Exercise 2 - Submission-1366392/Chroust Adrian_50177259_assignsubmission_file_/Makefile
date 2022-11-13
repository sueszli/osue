# Author: Adrian Chroust <e12023146@student.tuwien.ac.at>
# Date: 12.11.2021

# Name of program executable
PROGRAM = intmul

# Compiler flags
CC = gcc
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
COMPILE = $(CC) $(FLAGS)

.PHONY: all clean

all: $(PROGRAM)

clean:
	rm -rf src/*.o $(PROGRAM)

$(PROGRAM): src/intmul.o
	$(COMPILE) -o $@ $^

%.o: %.c
	$(COMPILE) -c -o $@ $<

src/intmul.o: src/intmul.c
