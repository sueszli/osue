/**
 * @file functions.c
 * @author Michael Trauner <e12019868@student.tuwien.ac.at>
 * @date 09.11.2021
 *
 * @brief Implements the functions for run-length encoding and counting digits of an integer
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "functions.h"

/** Digit counter
 * @brief Counts the number of digits of a given integer
 */
int countDigits(int n) {
    if (n < 0) {
        return 0;
    }
    if (n < 10) {
        return 1;
    }
    return 1 + countDigits(n / 10);
}

/** Run-length encoder
 * @brief Reads from a given input_file, run-length encodes the read data and returns the encoded data in output
 * and the number of read characters in input_size
 */
char* runlengthEncode(char *prog_name, FILE *input_file, char *output, int out_index, int *input_size) {
    int cur_char = fgetc(input_file), prev_char;
    int eq_char_count = 0;
    int is_eof = 0;
    do { // process one character for each iteration
        if (cur_char == '\r') { // Skip CR, needed for Unix to correctly count \n (new line- Windows: CR+LF | Unix: LF
            cur_char = fgetc(input_file);
        }

        if (feof(input_file)) { // process last character
            is_eof = 1;
        } else { // only count if a new character (not EOF) is processed
            (*input_size)++;
        }

        if ((is_eof && eq_char_count != 0) || (!is_eof)) { // Process character only if its not EOF or
            if (eq_char_count == 0) { // if no character has been read                      // if its not an empty file
                eq_char_count++;
                prev_char = cur_char;
            } else if (cur_char == prev_char) {
                eq_char_count++;
            } else { // a different character has been read and the current one is printed to the output
                int digitCount = countDigits(eq_char_count);

                int outputLength = (int) strlen(output) + 1; // +1 because of \0
                output = (char *) realloc(output, (sizeof(char)) *
                                                  (outputLength + digitCount + 1)); // + 1 for the character

                if (output == NULL) {
                    fprintf(stderr, " [%s] ERROR: realloc for output failed: %s\n", prog_name,
                            strerror(errno));
                    exit(EXIT_FAILURE);
                }

                output[out_index++] = (char) prev_char; // Add the current counted character to the output

                char *cur_count_char = malloc((sizeof(char)) * digitCount + 1); // +1 because of \0

                if (cur_count_char == NULL) {
                    fprintf(stderr, " [%s] ERROR: realloc for output failed: %s\n", prog_name,
                            strerror(errno));
                    exit(EXIT_FAILURE);
                }

                int ret_sprintf = sprintf(cur_count_char, "%d", eq_char_count);

                if (ret_sprintf < 0) {
                    fprintf(stderr, " [%s] ERROR: sprintf failed: %s\n", prog_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                for (int j = 0; j < digitCount; ++j) {
                    output[out_index++] = cur_count_char[j]; // Add the counted number to the output
                }
                output[out_index] = '\0'; // terminate the new allocated string

                eq_char_count = 1;
                prev_char = cur_char;
            }
            cur_char = fgetc(input_file);
        }
    } while (!is_eof);

    return output;
}