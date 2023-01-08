.PHONY: all clean

defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
default_mode = -std=c99 -pedantic -g -Wall $(defs) 
verbose_mode = -DDEBUG -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = 

all: client

client: client.o
	gcc -o client client.o $(libs)

client.o: client.c
	gcc $(default_mode) -c -o client.o client.c

clean:
	rm -rf *.o client
