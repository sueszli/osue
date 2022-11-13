# @file Makefile
 # @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 # @date 10.12.2021
 #
 # @brief Makefile for easier compilation and linking as well 
 #        as removing of unused object-files and executables 
 # 

CC = gcc
DEFS    = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 
CFLAGS  = -std=c99 -pedantic -Wall $(DEFS) -g

.PHONY: all clean

all: cpair
clean: 
	rm -rf *.o cpair
clean_without_exe:
	rm -rf *.o


cpair: cpair.o
	$(CC) $(CFLAGS) -lm -o cpair cpair.o

cpair.o: cpair.c cpair.h
	$(CC) $(CFLAGS) -c -o cpair.o cpair.c




