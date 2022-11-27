# @file Makefile
# @author Jannis Adamek (11809490)
# @date 2021-11-25
#
# @brief Makefile for exercise 2

CC      := gcc
DEFS    := -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
FLAGS   := -std=c99 -pedantic -Wall -g ${DEFS}
LIBS    := -lm -lpthread -lrt
NAME    := forksort
OBJECTS := ${NAME}.o

# Compile both the supervisior and the generator
all: ${OBJECTS}
	$(CC) ${FLAGS} -o ${NAME} $^ ${LIBS}
	chmod +x ${NAME}

%.o: %.c
	$(CC) ${FLAGS} -c -o $@ $<

clean:
	find -name '*.o' -delete
	find -name forksort -delete
