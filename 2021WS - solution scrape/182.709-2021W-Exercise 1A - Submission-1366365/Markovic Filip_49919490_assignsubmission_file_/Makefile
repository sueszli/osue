# @author Filip Markovic
# @programname mydiff

CC= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: mydiff mydiff.o 

mydiff: mydiff.o
	gcc -o mydiff mydiff.o

mydiff.o: mydiff.c
	gcc $(CC) -c mydiff.c

clean: 
	rm -rf mydiff.o mydiff