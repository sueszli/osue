# Program name: myexpand
# Author: Florian Spitzer, Student ID 00507913

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: myexpand

myexpand: myexpand.c
	gcc $(CFLAGS) -o myexpand myexpand.c

clean:
	rm myexpand
