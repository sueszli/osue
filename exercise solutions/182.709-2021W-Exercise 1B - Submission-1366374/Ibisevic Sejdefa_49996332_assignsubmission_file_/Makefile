#@makefile for Assignment 1B OSUE 2021 fb_arc_set
#@author: Sejdefa Ibisevic <e11913116@student.tuwien.ac.at>
#@date 14.11.2021
#
#@brief Makefile for fb_arc_set compiling
#

CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g 

LDLIBS = -lpthread -lrt

.PHONY: all clean
all: generator supervisor

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

generator: generator.o circBuff.o
	$(CC) $(LDLIBS) -o $@ $^

supervisor: supervisor.o circBuff.o
	$(CC) $(LDLIBS) -o $@ $^
	
# generates the tgz file
tar:
	tar -cvzf Assignment_1B_11913116.tgz Makefile *.c *.h
	
clean:
	$(RM) *.o supervisor generator Assignment_1B_11913116.tgz
