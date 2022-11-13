###
 # @file Makefile
 # @author Oliver Peter <e11915885@student.tuwien.ac.at>
 # @date 03.11.2021
 # @program name mygrep
 # @targets: mygrep
### 


CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

CC = gcc
OBJECTS = mygrep.o

.PHONY: all clean

all : mygrep

mygrep : $(OBJECTS)
	$(CC) -o $@ $^

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o : mygrep.c

clean :
	rm -rf *.o mygrep
