/**
 * @file mydiff.c
 * @author Simon Linder 11728330
 * @brief Compares two files and prints the amount of different characters per line.
 * @details Two files are compared line by line and the amount of different characters are printed.
 * If two lines are of different length then only the first n characters will be compared (where n is the length of the shorter line).
 * If differences are found then the line number and the number of different characters will be printed.
 * If the files are of different lengths then only the lines which both files have will be compared.
 * @date 9.11.2021
 * note: some parts were inspired by the lecture notes.
*/
#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

/**
 * @brief Compares two files and prints the amount of different characters per line.
 * @details Two files are compared line by line and the amount of different characters are printed.
 * If two lines are of different length then only the first n characters will be compared (where n is the length of the shorter line).
 * If differences are found then the line number and the number of different characters will be printed.
 * If the files are of different lengths then only the lines which both files have will be compared.
 * @param argc is the argument counter.
 * @param argv is the argument vector.
 * @return Returns EXIT_SUCCESS if the function was able to run successfully. Else it will return EXIT_FAILURE.
 */
int main(int argc, char *argv[]) {
    char *output = NULL;
    int opt_i = 0;
    int c;
    /*get all the options and arguments and make sure all options are valid*/
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i': {
                opt_i++;
                break;
            }
            case 'o': {
                output = optarg;
                break;
            }
            default: {
                fprintf(stderr, "%s Invalid option. To run mydiff use \"mydiff [-i] [-o outfile] file1 file2\"\n",
                        argv[0]);
                _exit(EXIT_FAILURE);
            }
        }
    }
    /* Check if the number of arguments is correct*/
    if ((argc - optind) != 2) {
        fprintf(stderr,
                "%s amount of arguments should be 2. To run mydiff use \"mydiff [-i] [-o outfile] file1 file2\n",
                argv[0]);
        _exit(EXIT_FAILURE);
    }
    char *input1 = argv[optind];
    char *input2 = argv[optind + 1];
    FILE *file1;
    FILE *file2;
    FILE *outputFile;
    if ((file1 = fopen(input1, "r")) == NULL) {
        fprintf(stderr, "%s %s does not exist\n", argv[0], input1);
        _exit(EXIT_FAILURE);
    }
    if ((file2 = fopen(input2, "r")) == NULL) {
        fclose(file1);
        fprintf(stderr, "%s %s does not exist\n", argv[0], input2);
        _exit(EXIT_FAILURE);
    }
    char *str1 = NULL;
    char *str2 = NULL;
    size_t line1_buf_size = 0;
    size_t line2_buf_size = 0;
    int lineCounter = 0;
    int length1;
    int length2;
    int shorterLength;
    int differentCharacters;
    /* chooses to what file (or stdout) the output shall be written. */
    if (output != NULL) {
        outputFile = fopen(output, "w");
    } else {
        outputFile = stdout;
    }
    /* executes the following loop for each line that both files have*/
    while (((getline(&str1, &line1_buf_size, file1)) >= 0) && ((getline(&str2, &line2_buf_size, file2)) >= 0)) {
        lineCounter++;
        str1[strcspn(str1, "\n")] = 0;
        str2[strcspn(str2, "\n")] = 0;
        length1 = strlen(str1);
        length2 = strlen(str2);
        if (length1 < length2) {
            shorterLength = length1;
        } else {
            shorterLength = length2;
        }
        differentCharacters = 0;
        /* counts the number of different characters in the current line*/
        for (int i = 0; i < shorterLength; i++) {
            if (opt_i > 0) {
                if (0 != strncasecmp(&str1[i], &str2[i], 1)) {
                    differentCharacters++;
                }
            } else {
                if (0 != strncmp(&str1[i], &str2[i], 1)) {
                    differentCharacters++;
                }
            }
        }
        if (differentCharacters > 0) {
            fprintf(outputFile, "Line: %d, characters: %d\n", lineCounter, differentCharacters);
        }
        fflush(outputFile);
    }
    fclose(file1);
    fclose(file2);
    free(str1);
    free(str2);
    if (outputFile != stdout) {
        fclose(outputFile);
    }
    _exit(EXIT_SUCCESS);
}
