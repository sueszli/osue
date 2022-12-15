.PHONY: all clean

# also see: https://gcc.gnu.org/onlinedocs/gcc-4.3.2/gcc/Warning-Options.html
defs = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
default_mode = -std=c99 -pedantic -g -Wall  $(defs) 
verbose_mode = -std=c99 -pedantic -g3 -Wall -Wextra -Wwrite-strings -Wconversion $(defs)
libs = -lm

all: intmul

intmul: intmul.o
	gcc -o intmul intmul.o $(libs)

intmul.o: intmul.c
	gcc $(verbose_mode) -c -o intmul.o intmul.c

clean:
	rm -rf *.o intmul