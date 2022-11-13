# @file Makefile
# @author David Posch e11817179@student.tuwien.ac.at
# @date 28.10.2021
#
# @brief Makefile for mycompress


all: main.o
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSDSOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -o mycompress main.o
	
main.o: main.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSDSOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c -o main.o main.c

clean:
	rm -rf *.o && rm -rf mycompress