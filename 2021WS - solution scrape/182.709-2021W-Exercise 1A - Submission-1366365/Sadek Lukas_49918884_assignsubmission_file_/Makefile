# @file Makefile
# @author Lukas Sadek (11813847)
# @date 14.11.2021
# @brief compiles and links everything

flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: ispalindrom

ispalindrom.o: ispalindrom.c
	gcc $(flags) -c ispalindrom.c

ispalindrom: ispalindrom.o
	gcc -o ispalindrom ispalindrom.o

clean:
	rm -f ispalindrom ispalindrom.o

