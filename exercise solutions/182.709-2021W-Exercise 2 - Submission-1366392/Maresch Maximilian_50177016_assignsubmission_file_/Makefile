# Author: Maximilian Maresch
# Program name: intmul

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: all clean

all : intmul

intmul : intmul.o addition.o child.o util.o
	gcc -o $@ $^

%.o : %.c
	gcc $(CFLAGS) -o $@ -c $< 

clean :
	rm -f intmul *.o