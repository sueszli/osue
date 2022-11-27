# Makefile Exersice 1B
# Daniel Blattner 12020646
# Compile programs supervisor and generator
CC = gcc
C_FLAGS = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LIBS = -lrt -lpthread
GENERATOR_NAME = generator
SUPERVISOR_NAME = supervisor
CIRCULARBUFFER_NAME = circularbuffer

.PHONY: clean all

all: $(SUPERVISOR_NAME) $(GENERATOR_NAME)

$(SUPERVISOR_NAME): $(SUPERVISOR_NAME).o $(CIRCULARBUFFER_NAME).o
	$(CC) $(C_FLAGS) $(LIBS) -o $@ $^

$(GENERATOR_NAME): $(GENERATOR_NAME).o $(CIRCULARBUFFER_NAME).o
	$(CC) $(C_FLAGS) $(LIBS) -o $@ $^

%.o : %.c
	$(CC) $(C_FLAGS) -c $^

run: 
	./$(SUPERVISOR_NAME)	

documentation: 
	doxygen ./Doxyfile

clean:
	rm -f $(GENERATOR_NAME) 
	rm -f $(SUPERVISOR_NAME) 
	rm -f *.o
	rm -rf html latex
