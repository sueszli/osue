FLAGS = -std=c99 -pedantic -Wall -Wextra -Wwrite-strings -Wconversion -g3 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: all clean

all: client

client: client.o
	gcc -o client client.o server.o

client.o: client.c common.h
	gcc -c -o client.o client.c $(FLAGS) 

clean:
	rm -f client client.o
