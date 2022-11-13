# Makefile Exersice 2
# Daniel Blattner 12020646
# Compile program intmul
CC = gcc
C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LIBS = -lrt -lpthread
PROGRAMM_NAME = intmul

.PHONY: clean all

all: $(PROGRAMM_NAME)

$(PROGRAMM_NAME): $(PROGRAMM_NAME).o 
	$(CC) $(C_FLAGS) $(LIBS) -o $@ $^

%.o : %.c
	$(CC) $(C_FLAGS) -c $^	

documentation: 
	doxygen ./Doxyfile

clean:
	rm -f $(PROGRAMM_NAME) 
	rm -f *.o
	rm -rf html latex
