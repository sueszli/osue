# @file Makefile
 # @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 # @date 14.11.2021
 #
 # @brief Makefile for easier compilation and linking as well 
 #        as removing of unused object-files and executables 
 # 

CC = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS  = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY: all clean

all: mycompress
clean: 
	rm -rf *.o mycompress
clean_without_exe:
	rm -rf *.o


mycompress: mycompress.o help_functions.o
	$(CC) $(CFLAGS) -o mycompress mycompress.o help_functions.o

mycompress.o: mycompress.c help_functions.h
	$(CC) $(CFLAGS) -c -o mycompress.o mycompress.c

help_functions.o: help_functions.c help_functions.h
	$(CC) $(CFLAGS) -c -o help_functions.o help_functions.c




