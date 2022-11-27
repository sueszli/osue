# Author: 	Ivaylo Bonev, matriculation number: 12025894
# Date:		05-Nov-2021
# Program:	mygrep

CFLAGS = 						\
	-std=c99 					\
	-pedantic 					\
	-Wall 						\
	-D_DEFAULT_SOURCE 			\
	-D_BSD_SOURSE 				\
	-D_SVID_SOURCE 				\
	-D_POSIX_C_SOURCE=200809L 	\
	-g 							\

.PHONY: all clean

all : mygrep

mygrep : mygrep.o
	gcc -o $@ $^

%.o : %.c
	gcc $(CFLAGS) -o $@ -c $<

clean :
	rm -f mygrep *.o *.out
