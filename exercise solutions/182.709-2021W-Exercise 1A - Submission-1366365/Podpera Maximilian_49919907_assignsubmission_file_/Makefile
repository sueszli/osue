# @author Max Podpera 11825326
# @program ispalindrom

CC = gcc
DEF = -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
CFLAGS = -Wall -g -std=c99 -pedantic $(DEF)
OBJECTS = main.o

# ohne file
.PHONY: all clean
all: ispalindrom

# recursive remove all .o files (force)
clean:
	rm -rf *.o ispalindrom

ispalindrom: $(OBJECTS)
	$(CC) -o $@ $^

# Allgemein für alle o files.
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c
isPalindrome.o: isPalindrome.c
# $() Variable
# $@ target
# $< first dependency
# $^ all dependencies

# main.o: main.c
#	$(CC) -c -o $@ $^