# @file Makefile
# @author Cveta Capova cveta.capova@tuwien.ac.at
# @date 12.11.2020

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS= -lpthread -lrt


.PHONY: all clean
all: function

function: function.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)


clean:
	rm -rf *.o function

	
	
