# @file Makefile
# @author Berke Akkaya 11904656
# @date 14.11.2021
# program: generator, supervisor


cStandard = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: supervisor generator


generator: generator.o
	gcc $(cStandard) -g -o generator generator.o -lrt -pthread 

generator.o: generator.c myheader.h
	gcc $(cStandard) -g -o generator.o -c generator.c 

supervisor: supervisor.o
	gcc $(cStandard) -g -o supervisor supervisor.o -lrt -pthread

supervisor.o: supervisor.c myheader.h
	gcc $(cStandard) -g -o supervisor.o -c supervisor.c 

clean: 
	rm -rf *.o supervisor generator