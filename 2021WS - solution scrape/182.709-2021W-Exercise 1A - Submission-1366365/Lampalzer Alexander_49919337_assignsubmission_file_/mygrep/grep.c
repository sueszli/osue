/**
 * @file grep.c
 * @author Alexander Lampalzer <e12023145@student.tuwien.ac.at>
 * @brief Source for the grep utility library
 *
 * See grep.h for documentation.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "grep.h"

int grep(const bool case_sensitive, const char *keyword, FILE *input, FILE *output) {
    char *_keyword = malloc(strlen(keyword) + 1); // strlen() does not contain null-terminator
    if (errno != 0) { return -1; } // Malloc failure
    strcpy(_keyword, keyword);

    // Convert keyword to lowercase, if necessary.
    if (!case_sensitive) {
        for (size_t i = 0; i < strlen(_keyword); ++i) {
            _keyword[i] = tolower((unsigned char) _keyword[i]);
        }
    }

    char *buffer = malloc(0);
    if (errno != 0) {
        free(_keyword);
        return -1;
    } // Malloc failure
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, input) != -1) {
        if (sizeof buffer < len) { buffer = realloc(buffer, len); }
        if (errno != 0) {
            free(_keyword);
            free(line);
            return -1;
        } // Malloc failure
        strcpy(buffer, line);

        // When case-insensitive, convert text in buffer to lowercase.
        if (!case_sensitive) {
            for (size_t i = 0; i < strlen(buffer); ++i) {
                buffer[i] = tolower((unsigned char) buffer[i]);
            }
        }

        // Match keyword against line
        if (strstr(buffer, _keyword) != NULL) {
            if (fputs(line, output) == EOF) {
                free(_keyword);
                free(buffer);
                free(line);
                return -1;
            }
        }
    }

    free(buffer);
    free(line);
    free(_keyword);

    if (errno != 0) {
        return -1;
    } else {
        return 0;
    }
}
