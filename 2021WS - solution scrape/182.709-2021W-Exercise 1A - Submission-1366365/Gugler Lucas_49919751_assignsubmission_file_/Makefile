# @file Makefile
# @author Lucas Gugler 12019849
#
# @brief Makefile for mycompress

CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

all: mycompress


%: %.c
	gcc $(CFLAGS) $< -o $@ -lm

clean:
	rm -f mycompress mycompress.o