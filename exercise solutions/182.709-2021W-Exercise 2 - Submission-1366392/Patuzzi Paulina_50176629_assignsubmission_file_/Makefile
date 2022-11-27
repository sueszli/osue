## @author Paulina Patuzzi (01607360)
## @program forksort
## @date 2021-12-08
## @brief this Makefile compiles and links the files for the forksort program

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -std=c99 -Wall -g -pedantic $(DEFS)
LDFLAGS = -lpthread -lrt

OBJECTS = forksort.o

all: forksort

forksort: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forksort.o: forksort.c forksort.h

clean: 
	rm -rf *.o forksort

.PHONY: all clean
