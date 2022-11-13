# @author Muhammad Adam Ferdin
# @details 11910541
# @brief Makefile for compiling the source files for mydiff
# @date 14.11.2021

DEFS: -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS: -Wall -g -std=c99 -pedantic $(DEFS)

.Phony: all clean

all: mydiff

mydiff:	main.o
	gcc $(LDFLAGS) -o mydiff main.o

main.o: main.c
	gcc $(CFLAGS) -c -o main.o main.c

clean: 
	rm -rf *.o mydiff


