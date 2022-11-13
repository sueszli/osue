# Makefile Exersice 1A
# Daniel Blattner 12020646
# Complie programm isplaindrom
CC = gcc
EXECUTABLE_NAME = ispalindrom
C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

.PHONY: clean all

all: $(EXECUTABLE_NAME)

$(EXECUTABLE_NAME): *.o
	$(CC) $(C_FLAGS) -o $@ $^

%.o : %.c
	$(CC) $(C_FLAGS) -c $^

run: 
	./$(EXECUTABLE_NAME)	

documentation: 
	doxygen ./Doxyfile

clean:
	rm -f $(EXECUTABLE_NAME) 
	rm -f *.o
	rm -rf html latex
