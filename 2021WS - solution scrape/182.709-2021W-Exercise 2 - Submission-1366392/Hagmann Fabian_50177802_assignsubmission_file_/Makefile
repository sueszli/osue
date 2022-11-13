# @file Makefile
# @brief Makefile for forksort (executable name: forksort)
# @author

CC = gcc
SOURCES = forksort.c
.PHONY: clean all
C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

EXECUTABLE_NAME = forksort

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): $(SOURCES)
	$(CC) $(C_FLAGS) -o $@ $^
%.o: %.c
	$(CC) $(C_FLAGS) -c $^
documentation:
	doxygen Doxyfile
clean:
	rm -f $(EXECUTABLE_NAME)
	rm -f *.o
	rm -rf html latex
