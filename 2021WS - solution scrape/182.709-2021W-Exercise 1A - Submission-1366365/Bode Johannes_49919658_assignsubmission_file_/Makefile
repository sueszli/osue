#@file Makefile
#@author Johannes Bode <e12025979@student.tuwien.ac.at>
#@date 10.11.2021

all: mycompress

CFLAGS= -g -std=c99 -pedantic -Wall $(SOURCE_FLAGS)

SOURCE_FLAGS= -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $^

mycompress : mycompress.o
	gcc $(LDFLAG) -o $@ $^


# remove all files produced during the build process
# -f (force) so no error on non existing files
clean: 
	rm -f mycompress *.o


# Phony (all and clean are not filenames) 
# to prevent conflict with other files
.PHONY: all clean
