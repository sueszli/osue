# @file Makefile
# @author Branislav Balvan <e12023159@student.tuwien.ac.at>
# @date 14.11.2021

FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: clean mycompress
	gcc -o mycompress mycompress.o
mycompress: mycompress.c
	gcc $(FLAGS) -c mycompress.c
	
clean:
	rm -rf *.o mycompress
