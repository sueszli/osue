CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: myexpand.c
	$(CC) $(CFLAGS) myexpand.c
	$(CC) myexpand.o -o myexpand

package:
	tar -cvzf myexpand.tgz myexpand.c makefile

clean:
	$(RM) myexpand.o
	$(RM) myexpand
	$(RM) *.tgz
