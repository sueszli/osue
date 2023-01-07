.PHONY: all clean

defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
default_mode = -std=c99 -pedantic -g -Wall  $(defs) 
verbose_mode = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = 

all: client server

client: client.o
	gcc -o client client.o $(libs)

server: server.o
	gcc -o server server.o $(libs)

client.o: client.c
	gcc $(verbose_mode) -c -o client.o client.c

server.o: server.c
	gcc $(verbose_mode) -c -o server.o server.c

clean:
	rm -rf *.o client server
