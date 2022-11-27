# @file Makefile
# @author Franziska Ludwig <e1100784@student.tuwien.ac.at>
# @date 31.10.2021

# @brief Makefile for mygrep

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

OBJECTS = main.o

.PHONY: all clean
all: mygrep

mygrep: $(OBJECTS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c 

clean: 
	rm -rf *.o mygrep 