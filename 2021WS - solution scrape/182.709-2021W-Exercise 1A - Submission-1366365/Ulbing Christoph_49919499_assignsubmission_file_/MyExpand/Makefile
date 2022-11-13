# author: Christoph Ulbing 12019872
# program name: myexpand


all: myexpand

compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

myexpand.o: myexpand.c
	gcc $(compile_flags) -c myexpand.c

myexpand: myexpand.o
	gcc -o myexpand myexpand.o

clean:
	rm -f myexpand myexpand.o