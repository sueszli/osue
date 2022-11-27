# @file Makefile
# @author Szymon Karabin <e11839439@student.tuwien.ac.at>
# @date 09.12.2021
#
# @brief Makefile for the programme.

all: cpair

cpair: cpair.o
	gcc -std=c99 -o cpair cpair.o -lm

cpair.o: cpair.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_SOURCE=200809L -g -c -o cpair.o cpair.c -lm

clean:
	rm *.o cpair