#!/bin/bash
# This scripts executes and analyzes your "main.c" file


# ----- compile to executable ----- 
# 1) minimal approach
gcc -o main main.c

# 2) default flags for OS UE course
gcc \
-std=c99 -pedantic -Wall -g \
-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L \
-o main main.c

# 3) most verbose output for debugging (requires installing gcc-12)
gcc-12 \
-std=c99 -pedantic -Wall -Werror -Wextra -g3 \
-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L \
-fsanitize=address \
-o main main.c


# ----- run executable ----- 
./main


# ----- run valgrind analysis (requires you to remove "-fsanitize=address") ----- 
valgrind --leak-check=full --show-leak-kinds=all -s ./main


# ----- clean everything up ----- 
rm -rf main.o main ./'-D_DEFAULT_SOURCE'
