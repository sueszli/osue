# @file Makefile
# @author Manuel Can Kaya 12020629
# @brief Makefile for assignment 1B Feedback arc set.
# @details Makefile containing the rules to compile and link the project Feedback arc set.
# @version 0.1
# @date 2021-11-13
# 
# @copyright Copyright (c) 2021
# 

# Configuration
CC = gcc -std=c99 -pedantic -Wall -pthread -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g


# Rules
all: supervisor generator

clean:
	rm -rf supervisor generator *.o *.tgz

tarball: supervisor.c generator.c feedback_arc_set.h feedback_arc_set.c Makefile
	tar -cvzf 1B_Feedback_Arc_Set_12020629.tgz supervisor.c generator.c feedback_arc_set.h feedback_arc_set.c Makefile

supervisor: supervisor.o feedback_arc_set.o
	$(CC) -o supervisor supervisor.o feedback_arc_set.o -lrt

generator: generator.o feedback_arc_set.o
	$(CC) -o generator generator.o feedback_arc_set.o -lrt

feedback_arc_set.o: feedback_arc_set.c
	$(CC) -c -o feedback_arc_set.o feedback_arc_set.c -lrt

supervisor.o: supervisor.c feedback_arc_set.h
	$(CC) -c -o supervisor.o supervisor.c -lrt

generator.o: generator.c feedback_arc_set.h
	$(CC) -c -o generator.o generator.c -lrt