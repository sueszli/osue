# AUTHOR: NICOLAS HRUBEC
# PROGRAM NAME: MYEXPAND_TD

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: myexpand.o
	gcc -o myexpand myexpand.o

myexpand.o: myexpand.c
	gcc $(CFLAGS) -c myexpand.c

clean:
	rm myexpand myexpand.o

