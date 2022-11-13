# @file Makefile
# @author Lucas Wolkersdorfer/11922587
# @date 11.12.2021
# @brief Assignement 2 intmul solution - Makefile
# 
# @details Makefile for intmul

CC = gcc

DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lm

.PHONY : all clean

all: intmul

generator: generator.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

supervisor: supervisor.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f intmul intmul.o

intmul.o: intmul.c intmul.h