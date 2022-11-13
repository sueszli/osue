# @file Makefile.mk
# @author Szymon Karabin <e11839439@student.tuwien.ac.at>
# @date 08.11.2021
#
# @brief Makefile for the programme.

all: mycompress

mycompress: mycompress.o
	gcc -std=c99 -o mycompress mycompress.o

mycompress.o: mycompress.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_SOURCE=200809L -g -c -o mycompress.o mycompress.c

clean:
	rm *.o mycompress