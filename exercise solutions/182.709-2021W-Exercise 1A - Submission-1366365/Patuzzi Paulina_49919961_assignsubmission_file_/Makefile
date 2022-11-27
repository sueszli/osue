## @author Paulina Patuzzi (01607360)
## @program mydiff
## @date 2021-11-02
## @brief this Makefile compiles and links the files for the mydiff program

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = mydiff.o

all: mydiff

mydiff: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
mydiff.o: mydiff.c mydiff.h

clean: 
	rm -rf *.o mydiff
.PHONY: all clean
