#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// function prototypes
_Bool isPalindrom(char *line);
char *trimWhitespace(char *line);
char *toLowerCase(char *line);

int main(int argc, char *argv[]) {
    char *line = " a a a a ";  // 5 whitespaces, 4 valid characters
    printf("%s \"%s\" %s --> \"%s\"\n\n", "The line", line, "results in",
           trimWhitespace(line));

    return EXIT_SUCCESS;
}

char *trimWhitespace(char *line) {
    char *p = line;
    int wcount = 0;
    while (*p != '\0') {
        if (isspace(*p)) {
            wcount++;
        }
        p++;
    }

    printf("%s %ld\n\n", "size of line:", sizeof(p));
    printf("%s %d\n\n", "found whitespaces:", wcount);

    char *out = malloc(sizeof(line) - count);

    return "out";
}

char *toLowerCase(char *line) {
    char *out = strdup(line);

    char *p = out;
    while (*p != '\0') {
        *p = tolower(*p);
        p++;
    }
    return out;
}

_Bool isPalindrom(char *line) {
    const size_t length = strlen(line);
    const size_t halfLength = length >> 1;

    if (length == 0) {
        return false;
    }

    for (size_t i = 0; i < halfLength; i++) {
        char left = line[i];
        char right = line[length - (1 + i)];
        if (left != right) {
            return false;
        }
    }
    return true;
}
