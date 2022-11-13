/*
 * @file mygrep.c
 * @author Alex Ploner 12024704
 * @date 23.10.2021
 * @brief simplified version of the Unix-command grep.
 * This program greps one or more input files and searches for a specified keyword (case sensitive or insensitive) and
 * outputs the matching lines to a specified file or to stdout
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* NAME_OF_PROGRAM;
static void mygrep(char *keyword, FILE *source, FILE *dest, int sensitivity);
static int contains(char *haystack, char *needle, int sensitivity);
static void ERROR_EXIT(char *strerror);
static void print_usage(void);


/*
 *
 * @brief checks if the options are valid and how many input files where specified.
 * Depending to that error messages are throw or the grep function is called.
 * @param argc Argument counter
 * @param argv Argument values
 */
int main (int argc, char** argv) {

    NAME_OF_PROGRAM = argv[0];

    int copt_i = 0;
    int copt_o = 0;
    int case_sensitivity = 1;
    char* output_file = NULL;

    char c;
    // check for options
    while ( (c = getopt(argc, argv, "io:")) != -1 ) {
        switch (c) {
            case 'i' :
                copt_i++;
                case_sensitivity = 0;
                break;
            case 'o' :
                output_file = optarg;
                copt_o++;
                break;
            case '?' :
                print_usage();
                break;
            default :
                print_usage();
                break;
        }
    }

    // options specified more than once
    if (copt_o > 1 || copt_i > 1) {
        print_usage();
    }

    FILE* dest;
    if (output_file == NULL) {
        dest = stdout;
    } else {
        dest = fopen(output_file, "w");
        if (dest == NULL) {
            fclose(dest);
            ERROR_EXIT(strerror(errno));
        }
    }

    int arg_index = optind;
    if (optind >= argc || argc == 1) {
        print_usage();
    }

    char* keyword = argv[arg_index];
    arg_index++;

    FILE* source = stdin;
    if (arg_index == argc) {
        mygrep(keyword, source, dest, case_sensitivity);
    } else {
        // call mygrep for every specified file
        while (arg_index < argc) {

            source = fopen(argv[arg_index], "r");
            if (source == NULL) {
                fclose(source);
                ERROR_EXIT(strerror(errno));
            }

            mygrep(keyword, source, dest, case_sensitivity);

            fclose(source);

            arg_index++;
        }
    }

    if (dest != NULL) {
        fclose(dest);
    }


    return 0;
}

/*
 * @brief prints the use of mygrep program.
 * @global NAME_OF_PROGRAM: name of the program
 */
static void print_usage(void) {
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", NAME_OF_PROGRAM);
    exit(EXIT_FAILURE);
}

/*
 * @brief reads line after line from the source file.
 * Then checks if the keyword, with the specified option (case sensitive/insensitive) is contained in a line.
 * If it's the case the line that contains the keyword is printed to the specified destination file.
 *
 * @param keyword: keyword to search for in the source.
 * @param source: file to read from and to search the keyword in it
 * @param dest: file to write the lines which contains the keyword
 * @param sensitivity: specifies if the search should be case sensitive or insensitive
 *
*/
static void mygrep(char *keyword, FILE *source, FILE *dest, int sensitivity) {

    char* line = NULL;
    size_t buf_length;

    while (getline(&line, &buf_length, source) != -1) {

        char* _line = strdup(line);
        char* _keyword = strdup(keyword);

        if (_line == NULL || _keyword == NULL) {
            ERROR_EXIT(strerror(errno));
        }

        int result = contains(_line, _keyword, sensitivity);

        if (result == 1) {
            if (fprintf(dest, "%s", line) < 0) {
                free(_keyword);
                free(_line);
                free(line);
                ERROR_EXIT("fprintf failed");
            }
        }

        free(_keyword);
        free(_line);
        free(line);
        buf_length = 0;

    }

    free(line);
}

/*
 * @brief prints the error message and than terminates the program.
 * @param strerror: error message text.
 * @global NAME_OF_PROGRAM: name of the program
 */
static void ERROR_EXIT(char *strerror) {
    fprintf(stderr, "[%s] ERROR: %s\n", NAME_OF_PROGRAM, strerror);
    exit(EXIT_FAILURE);
}


/*
 * @brief checks if a certain char sequence is the subsequence of another char sequence. (either case sensitive or insensitive)
 * @param haystack: char sequence in which to search for the needl
 * @param needle: char sequence to search for in the haystack
 * @param sensitivity: defines if the search has to be case sensitive or insensitive
 * @return 1 if the needle was found in the haystack
 * @return 0 if the needle was not found in the haystack
 */
static int contains(char *haystack, char *needle, int sensitivity) {

    if (sensitivity == 0) {

        for (int i = 0; i < strlen(haystack); ++i) {
            haystack[i] = tolower(haystack[i]);
        }

        for (int i = 0; i < strlen(needle); ++i) {
            needle[i] = tolower(needle[i]);
        }

    }

    if (strstr(haystack, needle) != NULL) {
        return 1;
    } else {
        return 0;
    }

}


