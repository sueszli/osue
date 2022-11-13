CC = gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: mycompress

mycompress: mycompress.c
	$(CC) -o mycompress mycompress.c

clean: 
	rm -rf mycompress.o mycompress