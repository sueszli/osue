#@author Daniel Atour Nissan 11810773    
#@brief replaces tab values in text files
#@date 29.10.2021
all: myExpand

cflags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g	-c

myExpand.o: myExpand.c
	gcc $( cflags) -c myExpand.c

myExpand: myExpand.o
	gcc -o myExpand myExpand.o

clean:
	rm -f myExpand myExpand.o