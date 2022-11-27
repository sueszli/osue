# @Author: Markovic Predrag
# Program: myexpand

CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
FLAGS = -std=c99 -pedantic -Wall -g $(DEFS)

all: link

link: myexpand.o 
	$(CC) -o myexpand myexpand.o
	
myexpand.o: myexpand.c
	$(CC) $(FLAGS) -c -o myexpand.o myexpand.c

clean: 
	rm -rf *.o
	rm myexpand