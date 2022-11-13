
#
# @file ispalindrom.c
# @author Alina Grassauer 11905176
# @date 14.11.2021
# @brief BSUE Exercise 1a ispalindrom
# 



CC = gcc
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

OBJECTS = ispalindrom.o

.PHONY: all clean

all: ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


ispalindrom.o: ispalindrom.c


clean:
	rm -rf *.o ispalindrom
