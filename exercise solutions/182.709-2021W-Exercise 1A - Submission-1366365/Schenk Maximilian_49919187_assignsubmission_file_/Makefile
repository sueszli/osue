# @file Makefile
# @author Maximilian Schenk 11908528 <e11908528@student.tuwien.ac.at>
# @date 21.10.2020
#
# @brief Makefile for mycompress

all: mycompress.o
	gcc -o mycompress mycompress.o

mycompress.o: mycompress.c
	gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mycompress.c

clean: mycompress.o mycompress
	rm -rf *.o
	rm -rf mycompress

# Import for TUWEL Upload
release:
	tar -cvzf MyCompress.tgz mycompress.c Makefile