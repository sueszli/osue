/**
 * @file main.c
 * @author Alexander Lampalzer <e12023145@student.tuwien.ac.at>
 * @date 31.10.2021
 *
 * @brief main module and entrypoint of mygrep
 * @details
 *
 * Matches a keyword against a stream or several files.
 * Matching lines are forwarded to a file or to a stream.
 *
 * @arg -i Toggle case sensitivity to be insensitive
 * @arg -o outfile Forward result stream to a file
 * @arg keyword Keyword to match input against
 * @arg filename Name of the file(s) to be read GREPed
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "grep.h"

/**
 * Internal utility function for grepping a file.
 * Most parameters, besides filename, are forwarded to grep().
 * See grep.h for more documentation.
 *
 * @param case_sensitive Match case sensitive
 * @param keyword Keyword to match against
 * @param filename File to be read
 * @param output Result Stream
 *
 * @return
 * On success, a zero status code is returned.
 * Otherwise, a non-zero status code is returned and errno is set.
 */
int grep_file(const bool case_sensitive, const char *keyword, const char *filename, FILE *output) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1;
    }

    if (grep(case_sensitive, keyword, fp, output) != 0) {
        fclose(fp);
        return -1;
    };

    fclose(fp);
    return 0;
}

/**
 * Entry point of mygrep
 *
 * @brief
 * Usage: mygrep [-i] [-o outfile] keyword [file...]
 *
 * @details
 * See beginning of main.c for in-depth explaination.
 *
 * @return
 * On success: EXIT_SUCCESS
 * Otherwise, EXIT_FAILURE
 *
 */
int main(int argc, char *argv[]) {
    int opt;
    bool case_sensitive = true;
    FILE *out = stdout;

    // Process (short) opt arguments.
    while ((opt = getopt(argc, argv, "hi::o:")) != -1) {
        switch (opt) {
            case 'i':
                case_sensitive = false;
                break;

            case 'h':
                fprintf(stdout, "Usage: mygrep [-i] [-o outfile] keyword [file...]\n");
                exit(EXIT_SUCCESS);

            case 'o':
                out = fopen(optarg, "w");
                if (out == NULL) {
                    fprintf(stderr, "%s: %s: ", argv[0], optarg);
                    perror("");
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                fprintf(stdout, "Usage: mygrep [-i] [-o outfile] keyword [file...]\n");
                exit(EXIT_FAILURE);
        }
    }

    int index = optind;
    char *keyword;

    // Read non-option argument keyword
    if (index < argc) {
        keyword = argv[index];
        index += 1;
    } else {
        fprintf(stderr, "No keyword specified.\n");
        fprintf(stdout, "Usage: mygrep [-i] [-o outfile] keyword [file...]\n");
        fclose(out);
        exit(EXIT_FAILURE);
    }

    // When no remaining arguments -> read from use stdin
    if (index == argc) {
        if (grep(case_sensitive, keyword, stdin, out) != 0) {
            fprintf(stderr, "%s: %s: \n", argv[0], argv[index]);
            perror("");
            fclose(out);
            exit(EXIT_FAILURE);
        }
    }

    // Remaining arguments: filenames
    for (; index < argc; index++) {
        if (grep_file(case_sensitive, keyword, argv[index], out) != 0) {
            fprintf(stderr, "%s: %s: ", argv[0], argv[index]);
            perror("");
            fclose(out);
            exit(EXIT_FAILURE);
        }
    }

    fclose(out);
    exit(EXIT_SUCCESS);
}
