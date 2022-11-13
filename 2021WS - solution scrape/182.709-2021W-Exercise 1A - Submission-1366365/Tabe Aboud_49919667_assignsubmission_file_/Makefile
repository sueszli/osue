# @file Makefile
# @author Aboud Tabe, E-Mail e1529106@student.tuwien.ac.at
# @date 14.11.2021
# @brief Makefile
#
# Program name: ispalindrom

# config

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all-fast: CFLAGS += -Ofast
all-fast: all

.PHONY: all clean
all: ispalindrom

ispalindrom: ispalindrom.o
	gcc ispalindrom.o -o ispalindrom

ispalindrom.o:
	gcc $(CFLAGS) ispalindrom.c

clean:
	rm -f *.o
	rm -f ispalindrom

fresh: clean ispalindrom
