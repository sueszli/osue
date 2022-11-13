/**
 * @brief mydiff: Compare two files
 *
 * @details
 * The `mydiff` program compares a file line-by-line and, for each differing line,
 * prints the number of differing characters.
 *
 * If the lines differ in length, excess characters of the longer line are silently ignored.
 * If the files have different numbers of lines, extra lines in the longer file are silently ignored.
 *
 * Normally, output is printed to stdout. This can be overridden with the `-o OUTFILE` option.
 *
 * Comparison is case-sensitive, unless `-i` is passed, in which case comparison is case-insensitive.
 *
 * @author Martin Formanek <e11708470@student.tuwien.ac.at>
 * @date 2021-11-14
 */

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

/**
 * @brief Prints usage of the program.
 */
void usage(void) {
    fprintf(stderr, "Usage: mydiff [-i] [-o OUTFILE] FILE1 FILE2\n");
}

/**
 * @brief Compare two strings.
 *
 * @param str1  First string to compare
 * @param str2  Second string to compare
 * @param len   ength to compare
 * @param cs    Whether or not the comparison shall be case-sensitive.
 * @return  Number of characters the strings differ by
 *
 * @details
 * Compares `str1` to `str2`. Exactly `len` characters will be compared.
 *
 * If `case_senstive` is true, comparison will be case-sensitive.
 * Otherwise, comparison will be case-insensitive.
 *
 * The number of different characters is counted and returned.
 */
int n_compare_strings(char* str1, char* str2, ssize_t len, bool cs) {
    int difference = 0;

    for (int i = 0; i < len; i++) {
        if (cs && str1[i] != str2[i]) {
            difference++;
        } else if (!cs && tolower(str1[i]) != tolower(str2[i])) {
            difference++;
        }
    }

    return difference;
}


/**
 * @brief Reads a line using POSIX `getline(3)`, handling errors and stripping the newline.
 *
 * @param linebuf   Buffer to read into
 * @param buflen    Length of the buffer
 * @param infile    File to read from
 * @param linelen   Will be set to the length of the line (without newline)
 * @param success   Will be set to `false` if an error occurs
 * @return  Whether or not the program should continue reading from the file
 *
 * @details
 * Reads a line from a file.
 *
 * `linebuf`, `buflen` and `infile` are passed without modification to `getline(3)`.
 * See `man 3 getline` for details.
 *
 * `linelen` will be set to the length of the line, as returned by `getline`.
 * However, if the last character read into the buffer is '\n' (i.e., for every
 * line except the last one), `linelen` will be decremented by 1.
 * This has the effect of trimming the line to exclude the '\n' character.
 *
 * Errors are handled by the function. If an error occurs, an error message is printed,
 * and the `success` parameter is set to `false`.
 *
 * If an error occurs, or the end of file is reached, this function returns `false`
 * to signal that the program should stop reading the file.
 * Otherwise, this function will return `true`.
 */
bool read_line(char **linebuf, size_t *buflen, FILE *infile, ssize_t *linelen, bool *success) {
    *linelen = getline(linebuf, buflen, infile);
    if (*linelen == -1) { // Cannot read line, EOF or error
        if (ferror(infile)) { // It's an error
            fprintf(stderr, "mydiff: Error reading from input file 1: %s", strerror(errno));
            *success = false;
        }
        return false;
    }

    // We do not want to compare the line-ending character
    if ((*linebuf)[*linelen-1] == '\n') {
        *linelen = *linelen - 1;
    }

    return true;
}

/**
 * @brief Returns the lesser of the two values.
 *
 * @param a  The first value to compare
 * @param b  The second value to compare
 * @return  The lesser of `a` and `b`.
 */
ssize_t min(ssize_t a, ssize_t b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

/**
 * @brief Compare two files, printing the differences to `stdout`.
 *
 * @param in1   One of the files to compare
 * @param in2   The other file to compare
 * @param cs    Whether or not the comparison should be case-sensitive.
 * @return  `false` if errors were encountered, `true` otherwise.
 *
 * @details
 * Compare the files `in1` and `in2`.
 *
 * Both files are processed line-by-line.
 *
 * For each line, the characters of the line are compared.
 * If there are differences, the number of differences is printed.
 *
 * If the lines differ in length, extra characters of the longer line are ignored.
 *
 * If one file has more lines than the other, the extra lines of the longer file are ignored.
 *
 * The number of different characters is printed to `stdout`.
 * Any errors encountered by the function will be reported to `stderr`.
 *
 * If the function encounters errors, it will return `false`.
 * Otherwise, the function will return `true`.
 */
bool compare_files(FILE *in1, FILE *in2, bool cs) {
    char *line1 = NULL, *line2 = NULL;
    size_t line1_buf_len = 0, line2_buf_len = 0;
    ssize_t line1_len = 0, line2_len = 0;
    ssize_t lesser_len;
    int cmp_result;
    int line_no = 0;
    bool success = true;

    for (;;) {
        line_no++;

        if (!read_line(&line1, &line1_buf_len, in1, &line1_len, &success))
            break;

        if (!read_line(&line2, &line2_buf_len, in2, &line2_len, &success))
            break;

        lesser_len = min(line1_len, line2_len);

        cmp_result = n_compare_strings(line1, line2, lesser_len, cs);

        if (cmp_result != 0) {
            printf("Line: %d, characters: %d\n", line_no, cmp_result);
        }
    }

    free(line1);
    free(line2);

    return success;
}

/**
 * @brief Entry point into the program.
 *
 * @param argc  Number of arguments
 * @param argv  Values of arguments
 * @return `EXIT_FAILURE` if errors were encountered, `EXIT_SUCCESS` otherwise.
 *
 * @details
 * `getopt` is used to handle command line options.
 * See the module-level documentation of `mydiff.c` for details about which options are accepted.
 *
 * If output should be redirected to a file, `stdout` is reopened as that file.
 *
 * The input files are opened and passed to `compare_files`, which will compare the files.
 *
 * Any errors are printed to `stderr`.
 *
 * In addition, if any errors are encountered, `EXIT_FAILURE` is returned.
 * Otherwise `EXIT_SUCCESS` is returned.
 */
int main(int argc, char** argv) {
    int c;
    bool case_sensitive = true;

    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i':
                case_sensitive = false;
                break;
            case 'o':
                if (!freopen(optarg, "w", stdout)) {
                    fprintf(stderr, "mydiff: Error opening output file: %s\n", strerror(errno));
                    return EXIT_FAILURE;
                }
                break;
            case '?':
                if (optopt == 'o') {
                    fprintf(stderr, "mydiff: Error: Option -o requires an argument\n");
                } else {
                    fprintf(stderr, "mydiff: Error: Unknown option -%c\n", optopt);
                }
                usage();
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "mydiff: Internal error while parsing command line arguments.\n");
                usage();
                return EXIT_FAILURE;
        }
    }

    if (argc != optind + 2) {
        fprintf(stderr, "mydiff: Error: Wrong number of arguments.\n");
        usage();
        return EXIT_FAILURE;
    }

    char* infile_1_name = argv[optind];
    char* infile_2_name = argv[optind + 1];

    FILE* infile1 = fopen(infile_1_name, "r");
    if (infile1 == NULL) {
        fprintf(stderr, "mydiff: Error opening input file %s: %s", infile_1_name, strerror(errno));
        return EXIT_FAILURE;
    }

    FILE* infile2 = fopen(infile_2_name, "r");
    if (infile2 == NULL) {
        fprintf(stderr, "mydiff: Error opening input file %s: %s", infile_2_name, strerror(errno));
        return EXIT_FAILURE;
    }

    if (compare_files(infile1, infile2, case_sensitive)) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
