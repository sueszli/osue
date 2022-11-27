# @file Makefile
# @author Larissa Artlieb 
# @date 25.10.2021
# @brief Makefile for compiling the project
# @details 	This makefile defines which source files are needed for compilation.
#			It can also be used to clean up the project directory using "make clean".

CC = gcc
DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
OBJECTS = ispalindrom.o
.PHONY: all clean
all: ispalindrom
ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
ispalindrom.o: ispalindrom.c ispalindrom.h
clean:
	rm -rf *.o ispalindrom