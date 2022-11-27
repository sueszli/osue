# @name    myexpand
# @brief   A program that converts tabs to an appropriate amount of spaces in a file or the command line.
# @author  Lukas Pichler, 01617432
# @date    14.11.2021

CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: myexpand

myexpand: myexpand.o
	$(CC) -o myexpand myexpand.o

clean:
	rm -f *.o
	rm -f myexpand