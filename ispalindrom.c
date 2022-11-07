#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// region "macros"
#define error(s)                                     \
    fprintf(stderr, "%s: %s\n", s, strerror(errno)); \
    exit(EXIT_FAILURE);
// endregion

// region "function prototypes"
_Bool isPalindrom(char *line);
char *trim(char *line);
char *toLowerCase(char *line);
// endregion

int main(int argc, char **argv) {
    char *input = "never odd or even";
    printf("\"%s\" ", input);

    _Bool ignoreLetterCasing = true;
    _Bool ignoreWhitespaces = true;

    if (ignoreLetterCasing) {
        input = toLowerCase(input);
    }
    if (ignoreWhitespaces) {
        input = trim(input);
    }

    if (isPalindrom(input)) {
        printf("is a palindrom\n");
    } else {
        printf("is not a palindrom\n");
    }

    return EXIT_SUCCESS;
}

char *trim(char *line) {
    char *out = strdup(line);

    // copy valid chars to duplicate and add '\0'
    char *op = out;
    char *ip = line;
    while (*ip != '\0') {
        if (!isspace(*ip)) {
            *op = *ip;
            op++;
        }
        ip++;
    }
    *op = '\0';

    // free everything following the '\0' (optional)
    // https://stackoverflow.com/questions/74350465/how-to-free-memory-following-a-0-placed-somewhere-in-a-string-in-c
    char *newMemory = realloc(out, strlen(out) + 1);
    if (newMemory) {
        out = newMemory;
    } else {
        error("Realloc failed");
    }

    return out;
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
