#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// function prototypes
_Bool isPalindrom(char *line);

int main(int argc, char *argv[]) {
    char *line = "racecar";
    printf("%s%s --> %d\n\n", "The word ", line, isPalindrom(line));
    return EXIT_SUCCESS;
}

char *trim(char *line) {
    // remove all whitespaces
    return "out";
}

char *turnLowerCase(char *line) {
    // turn all chars to lower case
    return "out";
}

_Bool isPalindrom(char *line) {
    const size_t length = strlen(line);
    const size_t halfLength = length >> 1;

    for (size_t i = 0; i < halfLength; i++) {
        char left = line[i];
        char right = line[length - (1 + i)];
        if (left != right) {
            return false;
        }
    }
    return true;
}
