# @author daniel brauneis 12021357
# @programname ispalindrom
CC = gcc
C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

.PHONY: clean all documentation

all: ispalindrom

ispalindrom: *.c
	$(CC) $(C_FLAGS) -o $@ $^

documentation:
	doxygen Doxyfile

run:
	./ispalindrom

runi:
	./ispalindrom -i

runs:
	./ispalindrom -s

runis:
	./ispalindrom -i -s

clean:
	rm -f ispalindrom
	rm -f *.o
	rm -f -r latex html