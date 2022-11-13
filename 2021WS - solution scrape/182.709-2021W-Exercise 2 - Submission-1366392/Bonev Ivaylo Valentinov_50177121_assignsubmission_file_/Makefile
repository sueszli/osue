# Author: 	Ivaylo Bonev, matriculation number: 12025894
# Date:		05-Dec-2021
# Program:	forkFFT

# Compilation flags.
CFLAGS = 						\
	-std=c99 					\
	-pedantic 					\
	-Wall 						\
	-D_DEFAULT_SOURCE 			\
	-D_BSD_SOURSE 				\
	-D_SVID_SOURCE 				\
	-D_POSIX_C_SOURCE=200809L 	\
	-g 							\

# Linking options for using math functions.
LINKFLAGS =						\
	-lm							\

all : forkFFT header

forkFFT : forkFFT.o
	gcc -o $@ $^ $(LINKFLAGS)

%.o : %.c
	gcc $(CFLAGS) -o $@ -c $<

header : common.h

clean :
	rm -f forkFFT *.o