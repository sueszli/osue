# @file Makefile
# @author Lulu Wang 01633998
# @date 12.11.2021
# @brief Makefile for mycompress.c

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

all: mycompress

%: %.c
	$(CC) $(CFLAGS) $< -o $@

tar:
	tar -cvzf mycompress1a.tgz Makefile *.c
clean:
	rm -rf mycompress mycompress.o