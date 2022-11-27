/**
 * @brief main file - reads lines from files or stdin and tells if the line is a palindrome
 * @author Moritz Bauer, 0649647
 * @date 21.11.11
 */
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>    /* for getopt */
#include <errno.h>
#include <string.h>
#include <ctype.h>

#ifdef DEBUG
#define debug(out,msg) \
    (void) fprintf(out, "[%s:%d] " msg  "\n", __FILE__, __LINE__);
#define fdebug(out,fmt, ...) \
    (void) fprintf(out, "[%s:%d] " fmt  "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define debug(...)
#define fdebug(...)
#endif

/**
 * @brief gets a string without any whitespaces and tells if string is a palindrome (case sensitive)
 * @param string
 * @param size
 * @return 1 if palindrome else 0
 */
int is_palindrome(char *string, int size) {
    for (int i = 0; i < size / 2; i++) {
        if (string[i] != string[size - i - 1]) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief processes a line and tells if it is a palindrome
 * @note could be more performant with having to pointers at front and end of string and
 *          move pointers towards each other, but performance and elegance were not asked
 * @param out stream or file to output results
 * @param is_ignore_whitespaces if 1 ignores whitespaces [ \t], if 0 does NOT ignore ws
 * @param is_ignore_case if 1 ignores case, if 0 does NOT ignore ws
 * @param line the line to process
 */
void processLine(FILE *out, int is_ignore_whitespaces, int is_ignore_case, char *line) {

    // remark: could be more performant with having to pointers at front and end of string and
    // move pointers towards each other,
    // but performance and elegance were not asked

    int len = strlen(line);

    // use copy string (buffer) to place string to check for validation
    // count size of copy for palindrome check
    char *copy = malloc(len * sizeof(char));
    int count = 0;

    // sanitize line and
    // remove whitespaces and/or convert to lowercase if required
    // write into copy

    for (int i = 0; i < len; i++) {
        char c = line[i];

        if (c == '\n') {  // if last symbol is a \n replace with \0 to truncate

            line[i] = '\0';
            continue;
        } else if (is_ignore_whitespaces && (c == ' ' || c == '\t')) {   // if is_ignore_whitespaces then skip whitespaces

            continue;
        } else if (is_ignore_case) {
            // if is_ignore_case then toLower
            c = tolower(c);
        }
        copy[count++] = c;
    }

    fprintf(out, "%s is %sa palindrome\n", line, is_palindrome(copy,count) > 0 ? "" : "not ");
    free(copy);
}

/**
 * @brief read from in stream and write results to out
 * @param in stream or file to read lines from
 * @param out stream or file to output results
 * @param is_ignore_whitespaces if 1 ignores whitespaces [ \t], if 0 does NOT ignore ws
 * @param is_ignore_case if 1 ignores case, if 0 does NOT ignore ws
 */
void readLinesFromStream(FILE *in, FILE *out, int is_ignore_whitespaces, int is_ignore_case) {
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, in) != -1) {
        processLine(out, is_ignore_whitespaces, is_ignore_case, line);
    }
    free(line);
}

/**
 * @brief read from file and process lines
 * @param out stream or file to output results
 * @param is_ignore_whitespaces if 1 ignores whitespaces [ \t], if 0 does NOT ignore ws
 * @param is_ignore_case if 1 ignores case, if 0 does NOT ignore ws
 * @param filename path of file to read lines from
 * @param arg_0 the executable - only needed for error handling
 */
void readFromFile(FILE *out, int is_ignore_whitespaces, int is_ignore_case, char *filename, char *arg_0) {
    FILE *file;
    if ((file = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "[%s] ERROR: fopen %s failed: %s\n", arg_0, filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    readLinesFromStream(file, out, is_ignore_whitespaces, is_ignore_case);
    fclose(file);
}

/**
 * @brief read from stdin and process lines
 * @param out stream or file to output results
 * @param is_ignore_whitespaces if 1 ignores whitespaces [ \t], if 0 does NOT ignore ws
 * @param is_ignore_case if 1 ignores case, if 0 does NOT ignore ws
 * @param arg_0 the executable - only needed for error handling
 */
void readFromStdin(FILE *out, int is_ignore_whitespaces, int is_ignore_case) {
    readLinesFromStream(stdin, out, is_ignore_whitespaces, is_ignore_case);
}

/**
 * do all the stuff
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {

    // define options
    int is_ignore_whitespaces = 0;
    int is_ignore_case = 0;
    int opt_o = 0;

    //get args with getopt
    int c;
    FILE *out = stdout;
    while ((c = getopt(argc, argv, "sio:")) != -1) {
        switch (c) {
            case 's':
                is_ignore_whitespaces = 1;
                debug(stdout, "-s");
                break;
            case 'i':
                is_ignore_case = 1;
                debug(stdout, "-i");
                break;
            case 'o':
                opt_o = 1;
                if (optarg) {
                    fdebug(stdout, "-o %s", optarg);

                    if ((out = fopen(optarg, "w")) == NULL) {
                        fprintf(stderr, "[%s] ERROR: fopen %s failed: %s\n", argv[0], optarg, strerror(errno));
                        exit(EXIT_FAILURE);
                    }

                }
                break;
            case '?':
                fprintf(stderr, "unknown param: %c", c);
                exit(EXIT_FAILURE);
                break;

            default:
                fprintf(stderr, "Usage: %s [-i] [-s] [-o outfile] [file...]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // do the actual exercise
    //if input files are provided read from each file
    //else read from stdin
    int num_pos_args = argc - optind;
    if (num_pos_args > 0) {
        for (int i = optind; i < argc; ++i) {
            fdebug(stdout, "arg: %s\n", argv[i]);

            readFromFile(out, is_ignore_whitespaces, is_ignore_case, argv[i], argv[0]);
        }
    } else {
        readFromStdin(out, is_ignore_whitespaces, is_ignore_case);
    }

    if (opt_o) {
        fflush(out);
        fclose(out);
    }

    exit(EXIT_SUCCESS);
}
