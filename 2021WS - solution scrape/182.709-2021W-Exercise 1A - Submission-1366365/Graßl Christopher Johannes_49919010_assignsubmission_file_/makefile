# Programm: myexpand
# Author: Christopher Graßl (11816883)
# Date: 13.11.2021


FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

OBJECTS = myexpand.o

.PHONY: all clean

all: myexpand

myexpand: myexpand.o
	gcc -o myexpand myexpand.o

myexpand.o: myexpand.c
	gcc $(FLAGS) -c -o myexpand.o myexpand.c

clean:
	rm -f myexpand.o myexpand
