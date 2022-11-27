#    	@author = Abdullah TOY
#    	@date = November 12th 2021
#	 	@program = ispalindrom

compileFlags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: ispalindrom

ispalindrom.o: ispalindrom.c
	gcc $(compileFlags) -c ispalindrom.c

ispalindrom: ispalindrom.o
	gcc -o ispalindrom ispalindrom.o

clean:
	rm -f ispalindrom ispalindrom.o