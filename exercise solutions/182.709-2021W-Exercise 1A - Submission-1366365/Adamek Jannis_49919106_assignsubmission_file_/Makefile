# @file Makefile
# @author Jannis Adamek (11809490)
# @date 2021-11-13
#
# @brief Makefile for assignment 1A.

flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g

# Compile the program with all required flags and set the executable permition.
all:
	gcc ${flags} main.c -o mydiff
	chmod +x mydiff

# Run the program.
run: all
	./mydiff

# Remove the executable.
clean:
	find -name mydiff -delete
