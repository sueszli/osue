# @file Makefile
# @author Branislav Balvan <e12023159@student.tuwien.ac.at>
# @date 14.11.2021

 
FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: clean supervisor generator
	gcc -o supervisor supervisor.o -lrt -lpthread
	gcc -o generator generator.o -lrt -lpthread
supervisor: supervisor.c datastructs.h
	gcc $(FLAGS) -c supervisor.c

generator: generator.c datastructs.h
	gcc $(FLAGS) -c generator.c
	
clean:
	rm -rf *.o generator supervisor
