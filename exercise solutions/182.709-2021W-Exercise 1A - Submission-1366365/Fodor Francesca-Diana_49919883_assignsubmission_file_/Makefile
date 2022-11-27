#@file makefile
#@author Francesca Diana Fodor, 11808223
#@date 2021-11-05
#compiles and links given files


all: myexpand

compile_options = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

myexpand.o: myexpand.c
	gcc $(compile_options) -c myexpand.c

myexpand: myexpand.o
	gcc -o myexpand myexpand.o

clean:
	rm -f myexpand myexpand.o
