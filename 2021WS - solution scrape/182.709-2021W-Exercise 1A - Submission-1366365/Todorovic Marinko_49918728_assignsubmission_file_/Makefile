# @file Makefile
# @author Marinko Todorovic <e11809906@student.tuwien.ac.at>
# @date 12.11.2021
#
# @brief The makefile which compiles and links files
# 

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: ispalindrom

ispalindrom.o: ispalindrom.c ; gcc $(CFLAGS) -c ispalindrom.c

ispalindrom: ispalindrom.o ; gcc -o ispalindrom ispalindrom.o

# generates the tgz file with all .c and .h files
tar:
	tar -cvzf Task1A.tgz Makefile *.c *.in *.out

clean: ; rm -f ispalindrom ispalindrom.o