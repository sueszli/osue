/**
 * @file util.c
 * @author Markus Paoli 01417212
 * @date 2021/11/10
 *
 * @brief Implementation of the utility module.
 *
 * @details This module contains utility functions for use by other modules.
 *          It includes a function for reading a line from a stream and one for replacing
 *          all tabs in a line with spaces up to a specified column width.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/**
 * The read_line function
 */
char* read_line(FILE *in) {
    char *ret;
    int buffer_size = 1024;
    if ((ret = malloc(sizeof(*ret) * buffer_size)) == NULL) {
        return NULL;
    }
    char c;
    int i = 0;
    while ((c = (char)fgetc(in)) != EOF && c != '\n') {
        ret[i] = c;
        ++i;

        if (i == buffer_size - 2) {
            char *tmp;
            if ((tmp = malloc(sizeof(*tmp) * buffer_size)) == NULL) {
                return NULL;
            }
            memcpy(tmp, ret, i);

            buffer_size *= 2;
            if ((ret = realloc(ret, sizeof(*ret) * buffer_size)) == NULL) {
                return NULL;
            }
            memcpy(ret, tmp, i);
            free(tmp);
        }
    }
	// add the newline at the end of the line
    if (c == '\n') {
        ret[i] = '\n';
        ++i;
    }
	// terminate the string
	ret[i] = '\0';
    return ret;
}

/**
 * The replace_tabs function
 */
char* replace_tabs(char *str, long tabstop) {
	int sum = 0;
    // count number of tabs
    long len = (long) strlen(str);
    for (int i = 0; i < len; ++i) {
        if (str[i] == '\t') {
            ++sum;
        }
    }
    char *tmp;
    if ((tmp = malloc(sizeof(*tmp) * (len + 1))) == NULL) {
        return NULL;
    }
    memcpy(tmp, str, len+1);

    // new string needs at most this many characters
    long new_len = (len + 1) + sum * (tabstop - 1);

    if ((str = realloc(str, sizeof(*str) * new_len)) == NULL) {
        return NULL;
    }

    // insert spaces
    int num_extra = 0; // counts the number of extra chars added to the new string
    long pos;
    for (int i = 0; i < len; ++i) {
        if (tmp[i] == '\t') {
		    pos = tabstop * (((i+num_extra) / tabstop) + 1);
            long dif = pos - (i + num_extra);
            for (int j = 0; j < dif; ++j) {
                str[i+num_extra] = ' ';
                ++num_extra;
            }
            // we replace one '\t' with '\n' we don't need to count this as no extra chars are added.
            --num_extra;
		} else {
            str[i+num_extra] = tmp[i];
        }
	}
    str[len+num_extra] = '\0';
    free(tmp);
	return str;
}
