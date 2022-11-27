#@makefile for Assignment 2 OSUE 2021 intmul
#@author: Sejdefa Ibisevic <e11913116@student.tuwien.ac.at>
#@date 04.112.2021
#
#@brief Makefile for intmul compiling
#

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

LDLIBS = -lpthread -lrt

.PHONY: all clean

all: intmul

intmul: intmul.o
	$(CC) $(CFLAGS) -g -o intmul intmul.o

intmul.o: intmul.c
	$(CC) $(CFLAGS) -g -o intmul.o -c intmul.c
	
# generates the tgz file
tar:
	tar -cvzf Assignment2_intmul_11913116.tgz Makefile *.c

clean:
	$(RM) *.o intmul Assignment2_intmul_11913116.tgz
