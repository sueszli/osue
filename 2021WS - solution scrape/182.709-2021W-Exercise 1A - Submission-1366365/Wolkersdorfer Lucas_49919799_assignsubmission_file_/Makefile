# @file Makefile
# @author Lucas Wolkersdorfer/11922587
# @date 14.11.2021
# @brief Assignement 1A ispalindrom solution - Makefile
# 
# @details Makefile for ispalindrom
all: ispalindrom

CC = gcc

DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

ispalindrom.o: ispalindrom.c
	$(CC) $(CFLAGS) -c ispalindrom.c

ispalindrom: ispalindrom.o
	$(CC) -o ispalindrom ispalindrom.o

clean:
	rm -f ispalindrom ispalindrom.o