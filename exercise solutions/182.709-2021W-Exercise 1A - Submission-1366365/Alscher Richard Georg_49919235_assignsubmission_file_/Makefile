#/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
# * @module  mygrep                                                                      *
# *                                                                                      *
# * @author  Richard Alscher - 11775285                                                  *
# *                                                                                      *
# * @brief   Reimplements the functionality of the grep commandline tool                 *
# * @details This reimplementation of the grep commandline tool supports an input file,  *
# *          an output file as well as case insensitivity                                *
# *                                                                                      *
# * @date    14.11.2021                                                                  *
#\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CC = gcc
#DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lpthread -lrt

G_OBJECTS = mygrep.o

.PHONY: all clean rebuild
all: mygrep

mygrep: $(G_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

mygrep.o: mygrep.c

clean:
	rm -rf *.o mygrep

rebuild: clean mygrep