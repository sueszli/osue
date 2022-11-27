# @file makefile
# @author Mischa Szasz <e12025957@student.tuwien.ac.at>
# @date 13.11.2021
# This makefile allows for a problem-free compilation process for the ispalindrom program.

OBJ1	= ispalindrom.o
OBJ1OUT = ispalindrom
SOURCE	= ispalindrom.c
CC	 	= gcc
FLAGS	= -g -c -Wall -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: $(OBJ1)
	$(CC) -g $(OBJ1) -o $(OBJ1OUT)
	
ispalindrom.o: ispalindrom.c
	$(CC) $(FLAGS) ispalindrom.c -std=c99

clean:
	rm -f $(OBJ1OUT) $(OBJ1)