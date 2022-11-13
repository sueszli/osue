/**
 * @file mygrep.c
 * @author Philipp Slowak (01427655) <e1427655@student.tuwiena.ac.at>
 * @date 25.10.2021
 * 
 * @brief This program implements a reduced version of UNIX's grep.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *progname; /**< The program name */

/**
 * @brief Prints a usage message to stderr and exits the program.
 * 
 * @return Returns EXIT_FAILURE
 */
static void usage(void);

/**
 * @brief Copies \p in to \p out in lowercase letters.
 * @details Copies the specified amount of letters from the specified input
 * string to the specified output string. All letters are copied as lowercase,
 * if possible. Adds terminating null character ('\0') at the end of \p out, 
 * that is after \p len characters have been copied. Note that \p in must have
 * the size of \p len excluding the null character. 
 * 
 * @param in The string to be copied
 * @param out The string to paste in in lowercase letters
 * @param len The amount of caracters to be copied
 */
static void tolowercase(const char *in, char *out, size_t len);

/**
 * @brief Closes the specified file.
 * 
 * @param f The file to be closed
 * @return Returns 0 on success, EOF otherwise
 */
static int fileclose(FILE *f);

/**
 * @brief Prints all lines of the specified input containing the specified
 * keyword to the specified output.
 * 
 * @param keyword The keyword to search for
 * @param in The file to be read
 * @param out The file to be written
 * @param casesens True if case sensetivity matters, false otherwise
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
static int grep(const char *keyword, FILE *in, FILE *out, bool casesens);

/**
 * @brief The main program. Arguments are parsed using getopt. 
 * 
 * @param argc The argument count
 * @param argv The argument vector
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main(int argc, char **argv) {

    progname = argv[0];

    const char *outname = NULL;
    bool casesens = true;

    int c;
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
        case 'i':
            casesens = false;
            break;
        case 'o':
            // option -o occurs multiple times
            if (outname != NULL) {
                fprintf(stderr, "[%s] ERROR: Option -o may be used at most once\n", progname);
                usage();
            }
            outname = optarg;
            break;
        case '?':
            usage();
        default:
            // unreachable code, abort program
            assert(0);
        }
    }

    // no keyword
    if ((argc - optind) < 1) {
        fprintf(stderr, "[%s] ERROR: Keyword required\n", progname);
        usage();
    }

    const char *const keyword = argv[optind];
    
    // open output file
    FILE *output = stdout;
    if (outname != NULL) {
        output = fopen(outname, "w");
        if (output == NULL) {
            fprintf(stderr, "[%s] ERROR: Could not open file %s: %s\n", progname, outname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // no input file
    if ((argc - optind) == 1) {
        int result = grep(keyword, stdin, output, casesens);
        if (result == EXIT_FAILURE) {
            fileclose(output);
            exit(EXIT_FAILURE);
        }
    } else {
        for (int i = optind + 1; i < argc; i++) {
            // open input file
            FILE *input = fopen(argv[i], "r");
            if (input == NULL) {
                fprintf(stderr, "[%s] ERROR: Could not open file %s: %s\n", progname, argv[i], strerror(errno));
                fileclose(output);
                exit(EXIT_FAILURE);
            }
            int result = grep(keyword, input, output, casesens);
            if (result == EXIT_FAILURE) {
                fileclose(input);
                fileclose(output);
                exit(EXIT_FAILURE);
            }
            fclose(input);
        }
    }

    fileclose(output);
    exit(EXIT_SUCCESS);
}

static void usage(void) {
    fprintf(stderr,"[%s] Usage: %s [-i] [-o outfile] keyword [file...]\n", progname, progname);
    exit(EXIT_FAILURE);
}

static void tolowercase(const char *in, char *out, size_t len) {
    int i;
    for (i = 0; i < len; i++) {
        out[i] = tolower(in[i]);
    }
    // copy terminating null character ('\0')
    out[i] = in[i];
}

static int fileclose(FILE *f) {
    if ((f != stdout) && (f != stdin)) {
        return fclose(f);
    }
    return 0;
}

static int grep(const char *keyword, FILE *in, FILE *out, bool casesens) {
    
    char *line = NULL;
    size_t len = 0;

    char *keywordcpy = (char*) keyword;
    
    if (!casesens) {
        size_t klen = strlen(keyword);
        keywordcpy = malloc((klen * sizeof(char)) + 1);
        if (keywordcpy == NULL) {
            fprintf(stderr, "[%s] ERROR: Could not allocate memory: %s\n", progname, strerror(errno));
            return EXIT_FAILURE;
        }
        tolowercase(keyword, keywordcpy, klen);
    }

    int nread = 0;
    // init with NULL important for realloc (see realloc(3))
    char *linecpy = NULL;
    while ((nread = getline(&line, &len, in)) != -1) {
        if (!casesens) {
            linecpy = realloc(linecpy, nread * sizeof(char));
            if (linecpy == NULL) {
                fprintf(stderr, "[%s] ERROR: Could not allocate memory: %s\n", progname, strerror(errno));
                free(keywordcpy);
                free(line);
                return EXIT_FAILURE;
            }
            tolowercase(line, linecpy, nread);
        } else {
            linecpy = line;
        }
       
        if (strstr(linecpy, keywordcpy) != NULL) {
            fprintf(out, "%s", line);
        }
    }

    if (!casesens) {
        free(keywordcpy);
        free(linecpy);
    }

    free(line);

    return EXIT_SUCCESS;
}