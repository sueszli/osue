//
// Created on 2021-11-04 at 14:04.
// Created by andreasmerckel (coffeecodecruncher@gmail.com)
//

#include "ispalindrom.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

/** @defgroup Palindrom */

/** @addtogroup Palindrom
 * @brief Checks if specified character strings are palindromes.
 *
 * @details One or more filestreams can be specified.
 * Case and whitespace sensitivity can be specified.
 *
 * @author Andreas Merckel 00746397
 * @date November 2021
 *  @{
 */


static char* usage(char* name);
static bool is_palindrom(char* c_string,
                         bool ignore_case,
                         bool ignore_whitespace);

static void check_file(FILE* file_in,
                       FILE* file_out,
                       unsigned int ignore_case,
                       unsigned int ignore_whitespace);

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[]) {
    char* outfile_arg = NULL;
    unsigned int count_s = 0;
    unsigned int count_i = 0;
    unsigned int count_o = 0;
    const char* optstring = "sio:";
    int c;

    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch (c) {
            case 's': {
                ++count_s;
                break;
            }
            case 'i': {
                ++count_i;
                break;
            }
            case 'o': {
                ++count_o;
                outfile_arg = optarg;
                break;
            }
            case '?': {
                fprintf(stderr, "[%s] ERROR with args:\n", argv[0]);
                fprintf(stderr, "%s", usage(argv[0]));
                exit(EXIT_FAILURE);
            }
            default:assert(0 && "Invalid optstring");
        }
    }

    if (count_s > 1) {
        fprintf(stderr,
                "ERROR: [%s] (Too many '-s' args)\n",
                argv[0]);
        fprintf(stderr, "%s", usage(argv[0]));
        exit(EXIT_FAILURE);
    }

    if (count_i > 1) {
        fprintf(stderr,
                "ERROR: [%s] (Too many '-i' args)\n",
                argv[0]);
        fprintf(stderr, "%s", usage(argv[0]));
        exit(EXIT_FAILURE);
    }

    if (count_o > 1) {
        fprintf(stderr,
                "ERROR: [%s] (Too many '-o' args)\n",
                argv[0]);
        fprintf(stderr, "%s", usage(argv[0]));
        exit(EXIT_FAILURE);
    }

    FILE* file_out = NULL;

    if (count_o > 0) {
        file_out = fopen(outfile_arg, "w");
        if (file_out == NULL) {
            fprintf(stderr,
                    "ERROR: [%s,%s] (fopen)\n",
                    argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    const int count_file_args = argc - optind;

    if (count_file_args == 0) {
        check_file(NULL,
                   file_out,
                   count_i,
                   count_s);
    }
    else {
        for (int i = 0; i < count_file_args; ++i) {
            FILE* input_file = fopen(argv[optind + i], "r");

            if (input_file == NULL) {
                if (file_out != NULL) {
                    fclose(file_out);
                }
                fprintf(stderr,
                        "ERROR: [%s %s] (fopen)\n",
                        argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }

            check_file(input_file,
                       file_out,
                       count_i,
                       count_s);
            fclose(input_file);
        }
    }

    if (file_out != NULL) {
        fclose(file_out);
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Reads from a file to check for palindromes
 *
 * @detail Input file is read line by line. Output file can be specified
 *
 * @param file_in filestream to read from
 * @param file_out filestream to write to
 * @param ignore_case ignore case when checking palindrome
 * @param ignore_whitespace ignore whitespace when checking palindrome
 */
void check_file(FILE* file_in,
                FILE* file_out,
                unsigned int ignore_case,
                unsigned int ignore_whitespace) {

    size_t line_size = 2;
    char* line = malloc(line_size);

    while ((getline(
            &line,
            &line_size,
            file_in == NULL ?
            stdin : file_in)) != -1) {

        char* pos = strchr(line, '\n');
        *pos = '\0';

        const uint8_t test = is_palindrom(line,
                                          ignore_case,
                                          ignore_whitespace);

        file_out = (file_out == NULL ? stdout : file_out);
        fprintf(file_out, "%s %s\n",
                line,
                test ? "is a palindrom" : "is not a palindrom");
    }

    free(line);
}

/**
 * Returns a string that explains the usage of the program
 *
 * @param name The name in question
 * @return A usage information
 */
char* usage(char* name) {
    char* res = "";
    strcat(res, "Usage:\n\n");
    strcat(res, "%s [-s] [-i] [-o outfile] [file...]\n");
    strcat(res, name);
    strcat(res, "\t-o output is written to the specified file\n");
    strcat(res, "\t-s causes program to ignore whitespaces\n");
    strcat(res, "\t-i ignore case.\n");
    return res;
}

/**
 * Checks if the specified string is a palindrome.
 * Options are:
 *
 * @param c_string The string to check
 * @param ignore_case Option to have case sensitive check
 * @param ignore_whitespace Option to include whitespace
 * @return true if c_string is palindrome, false if not
 */
bool is_palindrom(char* c_string,
                  bool ignore_case,
                  bool ignore_whitespace) {

    int i = 0;
    unsigned long j = strlen(c_string) - 1;

    while (i < j) {
        if (ignore_whitespace) {
            while (c_string[i] == ' ') {
                ++i;
                if (i > j) {
                    return true;
                }
            }
            while (c_string[j] == ' ') {
                --j;
                if (i > j) {
                    return true;
                }
            }
        }

        if (ignore_case &&
            (tolower(c_string[i]) != tolower(c_string[j]))) {
            return 0;
        }
        else {
            if (c_string[i] != c_string[j]) {
                return false;
            }
        }

        ++i;
        --j;
    }
    return true;
}


/** @}*/
