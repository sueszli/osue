/**
 *  @file utils.c
 *  @author Markus Paoli 01417212
 *  @date 2021/12/08
 *
 *  @brief The implementation of the utils module of the program "forksort".
 *
 *  Documentation can be found in the associated header file "utils.h".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

/**
 * The read_line function
 */
char* read_line(int fd) {
    int buffer_size = 256;
    char *buffer = malloc(sizeof(*buffer) * buffer_size);
    if (buffer == NULL) {
        return NULL;
    }
    memset(buffer, '\0', sizeof (*buffer) * buffer_size);
    int quit = 0;
    char c;
    int total_read = 0;
    while (quit == 0) {
        int num_read = 0;
        int pos;
        for (pos = 0; pos < 1; ) {
            num_read = (int) read(fd, &c, 1 - pos);
            pos += num_read;
            // error
            if (num_read < 0) {
                return NULL;
            }
            // EOF encountered
            if (num_read == 0) {
                quit = 1;
                break;
            }
        }
        if (num_read == 0) {
            if (strcmp("", buffer) != 0) {
                if (buffer[total_read-1] == '\n') {
                    break;
                } else {
                    buffer[total_read] = '\n';
                    ++total_read;
                }
            }
        }
        if (num_read > 0) {
            buffer[total_read] = c;
            total_read += num_read;
            // end of line
            if (c == '\n') {
                break;
            }
            if (total_read == (buffer_size - 2)) {
                char *tmp = malloc(sizeof(*tmp) * buffer_size);
                if (tmp == NULL) { return NULL; }
                memcpy(tmp, buffer, total_read);
                buffer_size = buffer_size << 1;
                buffer = realloc(buffer, sizeof(*buffer) * buffer_size);
                if (buffer == NULL) { return NULL; }
                memcpy(buffer, tmp, total_read);
                free(tmp);
            }
        }

    }
    buffer[total_read] = '\0';
    return buffer;
}

/**
 * The write_line function
 */
int write_line(int fd, const char *line) {
    int pos, num_wrote;
    for(pos = 0; pos < strlen(line); ) {
        num_wrote = (int) write(fd, line + pos, strlen(line) - pos);
        if (num_wrote < 0) {
            return -1;
        } else {
            pos += num_wrote;
        }
    }
    return 0;
}

/**
 * The free_lines function
 */
void free_line(char *line) {
    free(line);
}
