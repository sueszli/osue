#/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
# * @module  forkFFT                                                                     *
# *                                                                                      *
# * @author  Richard Alscher - 11775285                                                  *
# *                                                                                      *
# * @brief   makefile for the feedback_ark_set algorithm                                 *
# * @details                                                                             *
# *                                                                                      *
# * @date    18.11.2021                                                                  *
#\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

CC = gcc
#DEFS = -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
DEFS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS = -lpthread -lrt -lm

F_OBJECTS = forkFFT.o

.PHONY: all clean rebuild
all: forkFFT

forkFFT: $(F_OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

forkFFT.o: forkFFT.c

clean:
	rm -rf *.o forkFFT

rebuild: clean forkFFT