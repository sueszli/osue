GCC = gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
.PHONY: all clean

all: myexpand

myexpand: myexpand.o
	gcc -o $@ $^

myexpand.o: myexpand.c myexpand.h
	$(GCC) -c -o $@ $<

clean: 
	rm -rf *.o myexpand