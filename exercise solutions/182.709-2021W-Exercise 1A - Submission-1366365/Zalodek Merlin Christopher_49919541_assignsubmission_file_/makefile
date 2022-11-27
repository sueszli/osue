## ----------------------------------------------------------------------
## Program: Assignment A – ispalindrom
## Author: Merlin Zalodek, 12019836
## ----------------------------------------------------------------------


CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c

all: ispalindrom.c
	$(CC) $(CFLAGS) ispalindrom.c
	$(CC) ispalindrom.o -o ispalindrom

ispalindrom: ispalindrom.c
	$(CC) $(CFLAGS) ispalindrom.c
	$(CC) ispalindrom.o -o ispalindrom

clean:
	rm *.o
	rm *.tgz

package:
	tar -cvzf Zalodek_Merlin_12019836_1A.tgz ispalindrom.c makefile

commit:
	tar -cvzf Zalodek_Merlin_12019836_1A.tgz ispalindrom.c makefile
	scp -r Zalodek_Merlin_12019836_1A.tgz y12019836@ssh.inflab.tuwien.ac.at:Documents/1A/
