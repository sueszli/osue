#/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
# * @module  fb_arc_set                                                                  *
# *                                                                                      *
# * @author  Richard Alscher - 11775285                                                  *
# *                                                                                      *
# * @brief   makefile for the feedback_ark_set algorithm                                 *
# * @details                                                                             *
# *                                                                                      *
# * @date    14.11.2021                                                                  *
#\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CC = gcc
#DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lpthread -lrt

S_OBJECTS = supervisor.o
G_OBJECTS = generator.o

.PHONY: all clean rebuild
all: supervisor generator

supervisor: $(S_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

generator: $(G_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

supervisor.o: supervisor.c
generator.o: supervisor.c

clean:
	rm -rf *.o supervisor generator

rebuild: clean supervisor generator