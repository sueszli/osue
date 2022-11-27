# Christoph Neubauer 12023172, 2021-11-21
CC = gcc
EXECUTABLE_NAME = forksort
C_FLAGS = -std=c99 -Wall -pedantic -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LINK_LIBS = -lrt -lpthread

.PHONY: clean all

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): forksort.o
	$(CC) $(C_FLAGS) $(LINK_LIBS) -o $@ $^


%.o: %.c
	$(CC) $(C_FLAGS) -c $^

documentation: *.c
	doxygen Doxyfile

clean:
	rm -f $(EXECUTABLE_NAME)
	rm -f *.o
	rm -rf html latex
