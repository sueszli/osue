#
# @file Makefile
# @author Mischa Szasz <e12025957>
# @date 10.12.2021
#
# @brief Makefile for program compilation.
# 
# This Makefile allows to compile the intmul.c program.

FLAGS = -g -c -Wall -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L

all: intmul

intmul: intmul.o
	gcc -o intmul intmul.o

intmul.o: intmul.c 
	gcc $(FLAGS) intmul.c

clean: 
	rm -rf *.o *.out intmul      