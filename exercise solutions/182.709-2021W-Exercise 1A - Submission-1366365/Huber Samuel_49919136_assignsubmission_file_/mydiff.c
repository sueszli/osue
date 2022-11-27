/**
 * module name: mydiff
 * @author      Huber Samuel 11905181
 * @brief       mydiff reads in two lines and compares them
 * @details     If two lines differ while comparing, the line number
 *              and amount of differing chars is printed
 *              option -o defines to which file the output should be written (stdout if not given)
 *              if option -i is given, the comparison is case insensitive, sensitive otherwise
 * @date        21.10.2021
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>

FILE *input1;   // first input file read
FILE *input2;   // second input file read
FILE *output;   // file, to which output will be written, stdout if not given

/**
 * @brief prints out usage message
 * @details printed message advises the user, on how to call program properly \n program exits afterwards
 * @param prog: name of the current program
 */
static void usage(char *prog);

/**
 * @brief closes used I/O file streams
 * @details checks if file streams are already closed \n
 *          closes them if not \n
 *          removes resources afterwards \n
 *          global variables used:\n
 *          'output', 'input1', 'input2'
 */
static void closeIO(void);

/**
 * @brief compares the lines of two input files given
 * @details if two lines differ while comparing, the line number \n
 *          and amount of differing chars is printed into given output \n
 *          if i-flag is given when calling program, comparison is not case sensitive \n
 *          comparison of the current lines stops, if no char is left in either one of them \n
 *          further characters in the respective other line are ignored \n
 *          global variables used:\n
 *          'output', 'input1', 'input2'
 * @param i_flag: if flag is set, comparison is not case sensitive
 */
static void fileCompare(int i_flag);


static void usage(char *prog) {
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", prog);
    exit(EXIT_FAILURE);
}

static void closeIO(void) {
    if (output != NULL) {
        fclose(output);
        output = NULL;
    }

    if (input1 != NULL) {
        fclose(input1);
        input1 = NULL;
    }

    if (input2 != NULL) {
        fclose(input2);
        input2 = NULL;
    }
}

static void fileCompare(int i_flag) {

    char currentChar_1;
    char currentChar_2;

    int row = 1;
    int diff = 0;

    while (((currentChar_1 = fgetc(input1)) != EOF) && ((currentChar_2 = fgetc(input2)) != EOF)) {

        if (currentChar_1 == '\n' || currentChar_2 == '\n') {
            if (diff > 0) {
                fprintf(output, "Line: %i, characters: %i\n", row, diff);
            }

            if (currentChar_1 != '\n') {
                while (currentChar_1 != '\n' && currentChar_1 != EOF) {
                    currentChar_1 = fgetc(input1);
                }

            } else if (currentChar_2 != '\n') {
                while (currentChar_2 != '\n' && currentChar_2 != EOF) {
                    currentChar_2 = fgetc(input2);
                }
            }
            row++;
            diff = 0;
        } else if (currentChar_1 != currentChar_2) {
            if (i_flag == 0) {
                diff++;
            } else {
                if (toupper(currentChar_1) != toupper(currentChar_2)){
                    diff++;
                }
            }

        }
    } if (diff > 0) {
      fprintf(output, "Line: %i, characters: %i\n", row, diff);
    }

}

/**
 *
 * @param argc
 * @param argv
 * @return boolean if
 */
int main(int argc, char *argv[]) {

    input1 = NULL;
    input2 = NULL;
    output = NULL;

    char *prog = argv[0];   // name of program

    int i_flag = 0;         // flag if option i was given

    char *o_arg = NULL;     //output-file name pointer
    int opt_o = 0;          //amount of output-files given

    int opt;                // option handling
    while ((opt = getopt(argc, argv, "io:")) != -1) {
        switch (opt) {
            case 'i':
                i_flag = 1;
                break;
            case 'o':
                if (opt_o > 0) {
                    fprintf(stderr, "[%s] ERROR: More than one output file given \n", prog);
                    usage(prog);
                }
                o_arg = optarg;
                opt_o++;
                break;
            case '?':
                fprintf(stderr, "[%s] ERROR: Wrong use of arguments \n", prog);
                usage(prog);
                break;
            default:        //should not be reachable -> defensive programming
                assert(0);
        }
    }

    // if no output file give, use stdout for output
    if (o_arg == NULL) {
        output = stdout;
    } else {
        output = fopen(o_arg, "w");
        if (output == NULL) {
            fprintf(stderr, "[%s] ERROR: fopen(%s) failed: %s\n", prog, o_arg, strerror(errno));
            closeIO();
            exit(EXIT_FAILURE);
        }
    }

    // handle input files
    if (optind + 2 != argc) {
        fprintf(stderr, "[%s] ERROR: Wrong use of arguments \n", prog);
        usage(prog);
    } else {
        input1 = fopen(argv[optind], "r");
        if (input1 == NULL) {
            fprintf(stderr, "[%s] ERROR: opening first input file failed: %s\n", prog, strerror(errno));
            closeIO();
            exit(EXIT_FAILURE);
        }
        input2 = fopen(argv[optind + 1], "r");
        if (input2 == NULL) {
            fprintf(stderr, "[%s] ERROR: opening second input file failed: %s\n", prog, strerror(errno));
            closeIO();
            exit(EXIT_FAILURE);
        }
    }

    fileCompare(i_flag);

    closeIO();
    exit(EXIT_SUCCESS);
}
