# Author: Hao Hao Ying <e01625773@student.tuwien.ac.at>
# Program name: myexpand

CC = gcc
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
.PHONY: all clean
all: myexpand
	

myexpand: myexpand.o
	$(CC) -o myexpand myexpand.o

myexpand.o: myexpand.c
	$(CC) $(FLAGS) myexpand.c


clean:
	rm -f *.o myexpand
