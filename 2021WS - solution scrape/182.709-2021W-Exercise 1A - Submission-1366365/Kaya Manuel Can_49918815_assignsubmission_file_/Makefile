# @file Makefile
# @author Manuel Can Kaya 12020629 <e12020629@student.tuwien.ac.at>
# @date 07.11.2021
#
# @brief Makefile for mycompress assignment.

# Configuration
CC = gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

# Rules
all: mycompress

clean:
	rm -rf mycompress *.o *.tgz

mycompress: mycompress.o
	$(CC) -o mycompress mycompress.o

mycompress.o: mycompress.c
	$(CC) -c mycompress.c

tarball: mycompress.c Makefile
	tar -cvzf 1A_MyCompress_12020629.tgz mycompress.c Makefile
