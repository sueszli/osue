.PHONY: all clean

# also see: https://gcc.gnu.org/onlinedocs/gcc-4.3.2/gcc/Warning-Options.html
defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
default_mode = -std=c99 -pedantic -g -Wall  $(defs) 
verbose_mode = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs =

all: ispalindrome

ispalindrome: ispalindrome.o
	gcc -o ispalindrome ispalindrome.o

ispalindrome.o: ispalindrome.c
	gcc $(verbose_mode) -c -o ispalindrome.o ispalindrome.c

clean:
	rm -rf *.o ispalindrome