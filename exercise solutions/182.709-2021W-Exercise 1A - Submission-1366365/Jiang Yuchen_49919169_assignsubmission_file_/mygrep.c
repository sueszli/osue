/**
 * @file mygrep.c
 * @author Jiang Yuchen 12019845
 * @date 27.10.2021
 * @brief mygrep program
 **/

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

/**
 * Main method
 * @brief This method contains all the functionalities of the mygrep program
 * @param argc Stores the number of arguments
 * @param argv Array of arguments
 * @return EXIT_SUCCESS
 */
int main(int argc, char** argv) {
    int c;
    char *o_arg = NULL;
    int insensitive = 0;
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'o':
                o_arg = optarg;
                break;
            case 'i':
                insensitive = 1;
                break;
            case '?':
                fprintf(stderr, "[%s] ERROR: Arguments wrong! Usage: mygrep [-i] [-o outfile] keyword [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
            default:
                assert(0);
        }
    }
    int numPosArg = argc - optind;
    char *keyword;
    if (numPosArg == 0) {
        fprintf(stderr, "[%s] ERROR: Arguments wrong! Usage: mygrep [-i] [-o outfile] keyword [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        keyword = argv[optind];
    }

    int keyWordLength = strlen(keyword);
    optind++;
    numPosArg--;

    int standard = 0;
    if (numPosArg == 0) {
        numPosArg = 1;
        standard = 1;
    }

    FILE *output;
    if (o_arg == NULL) {
        output = stdout;
    } else {
        output = fopen(o_arg, "a");
    }
    if (output == NULL) {
        fprintf(stderr, "[%s] fopen (output) failed: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    while (numPosArg >= 1) {
        FILE *input;
        if (standard) {
            input = stdin;
        } else {
            input = fopen(argv[optind], "r");
        }
        optind++;
        numPosArg--;
        if (input == NULL) {
            fclose(output);
            fprintf(stderr, "[%s] fopen (input) failed: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        char *line = NULL;
        int contains;
        while (feof(input) == 0) {
            size_t bufsize = 0;
            size_t length;
            length = getline(&line, &bufsize, input);
            if (length == -1) {
                fprintf(stderr, "[%s] geline failed: %s\n", argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            } else {
                for (int i = 0; i < length; ++i) {
                    contains = 1;
                    for (int j = 0; j < keyWordLength; ++j) {
                        if (insensitive == 1) {
                            if (toupper(line[i+j]) != toupper((char) keyword[j])) {
                                contains = 0;
                            }
                        } else {
                            if (line[i+j] != (char) keyword[j]) {
                                contains = 0;
                            }
                        }
                    }
                    if (contains == 1) {
                        fputs(line, output);
                        fflush(output);
                        break;
                    }
                }
            }
            free(line);
            line = NULL;
        }
        fclose(input);
    }
    fclose(output);
    return EXIT_SUCCESS;
}