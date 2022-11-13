 # 
 # 
 # @file Makefile
 # @author Alina Grassauer 11905176
 # @date 07.12.2021
 # @brief BSUE Exercise 2 intmul
 # @details 
 # 
 # 


CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = intmul.o

.PONY : all clean

all: intmul

intmul: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	

intmul.o: intmul.c intmul.h

clean:
	rm -rf *.o intmul

