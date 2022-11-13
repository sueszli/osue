/**
 * @file mygrep.c
 * @author Jan Leszczyk 12019835
 * @brief A reduced version of the Unix-command grep.
 * @details This program one or multiple lines from an input file and
 * compares the individual strings with a keyword. Should there be a 
 * match the line will get printed in an ouput file.
 * @date 2021-10-31
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

/**
 * @brief A Message for reminding the user of the correct usage syntax.
 * @details Exits the program with EXIT_FAILURE if the execution command is not of the
 * form. 
 * Uses global variable PROGRAM_NAME.
 */
static void USAGE(void);

/**
 * @brief Error message.
 * @details Writes a error message to stdout with information about the failed function
 * and errno. Uses global variable PROGRAM_NAME.
 * @param msg Personalised message about the failed function.
 * @param error_str Error message found in the corresponding errno.
 */
static void ERROR_EXIT(char *msg, char *error_str);

/**
 * @brief Searches for the keyword in the input file.
 * @details The function scans the input file line by line for the keyword. If the
 * keyword is found in a lines, this line gets printed into the output file.
 * If the case sensitivity flag is set to 1 the function will first of all execute
 * str_tolower() on both the keyword as well as the currently got line.
 * @param infile The input file, in which the keyword is being searched.
 * @param outfile The outputfile to which matched lines get printed.
 * @param case_sens The case sensitivity flag.
 * @param keyword The keyword with which line are compared.
 */
static void mygrep(FILE* infile, FILE* outfile, int case_sens, char* keyword);

/**
 * @brief Sets a string to lowercase.
 * @details While iterating through the whole string, each charakter will be set
 * to lowercase with tolower().
 * @param s The string.
 */
static void str_tolower (char* s);

/**
 * @brief Sets up the signal handling.
 * @details Sets up signal handling and executes handle_signal() if either SIGINT or
 * SIGTERM is received.
 */
static void signal_handling(void);

/**
 * @brief Shuts down generator SIGINT or SIGTERM is recieved.
 * @details Uses global variable quit.
 */
static void handle_signal(int signal);

/**
 * @brief The name of the executed Program.
 */
static char* PROGRAM_NAME;

/**
 * @brief A variable to signal the termination of the process.
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief Executes the my grep program.
 * @details The function reads in the arguments and checks with getopt(), wether 
 * there are any of the optional options defined. Should an output- and/or 
 * input files be defined then it opens them and begins comparing line by line.
 * The function end if either all of the input files have been read or the program
 * gets interrupted. 
 * Uses global variable PROGRAM_NAME.
 */
int main (int argc, char *argv[]) {
    PROGRAM_NAME = argv[0];

    signal_handling();

    // Output file is set to stdout by default
    FILE *outfile = stdout;
    // Input file is set to stdin by default
    FILE *infile = stdin;

    // Input Options
    int case_sens = 1;
    int i_used = 0;
    int o_used = 0;

    int opt;
    while((opt = getopt(argc, argv, "io:")) != -1) {
        switch (opt) {
        case 'i':
            case_sens = 0;
            i_used++;
            break;
        
        case 'o':
            outfile = fopen(optarg, "w");
            o_used++;
            break;

        case '?':
            USAGE();
            break;

        default:
            USAGE();
            break;
        }
    }

    // Checks whether the output file could be opened
    if (outfile == NULL) {
        ERROR_EXIT("Could not open output file", strerror(errno));
    }
    
    // Checks if (at least) one of the options has been used more then once 
    if (i_used > 1) {
        USAGE();
    }

    if (o_used > 1) {
        USAGE();
    }

    // The program should receive at least two arguments
    if (argc < 2) {
        USAGE();
    }

    char *keyword = argv[optind];
    if (keyword == NULL) {
        USAGE();
    }
    
    int infile_ind = optind + 1;

    // If there are no more arguments after the keyword the program starts
    // with the default input file
    if (argc - infile_ind <= 0) {
        mygrep(infile, outfile, case_sens, keyword); 
    }
    // Otherwise it looks at all the input files individually 
    else {
        for (; infile_ind < argc; infile_ind++) {
            infile = fopen(argv[infile_ind], "r");

            if (infile == NULL) {
                    ERROR_EXIT("Could not open input file", strerror(errno));
            }

            mygrep(infile, outfile, case_sens, keyword);

            fclose(infile);
        }
    }
    
    fclose(outfile);

    return EXIT_SUCCESS;
}

static void str_tolower (char* s) {
    for(; *s != '\0'; ++s) {
        *s = tolower(*s);
    }
}

void mygrep(FILE* infile, FILE* outfile, int case_sens, char* keyword) {
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (!quit) {
        if ((nread = getline(&line, &len, infile)) == -1) break;

        char* haystack = strdup(line);

        if (case_sens == 0) {
            str_tolower(haystack);
            str_tolower(keyword);
        }

        if (strstr(haystack, keyword) != NULL) {
            fprintf(outfile, "%s", line);
        }    
    }
}

static void signal_handling(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    if (sigaction(SIGINT, &sa, NULL) == -1 || sigaction(SIGTERM, &sa, NULL) == -1) {
        ERROR_EXIT("Could not execute signal handler correctly", strerror(errno));
    }
}

static void handle_signal(int signal) {
    quit = 1;
}

void USAGE(void) {
    fprintf(stderr, "Correct usage of %s: %s [-i] [-o outfile] keyword [file...] \n", PROGRAM_NAME, PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

void ERROR_EXIT(char *msg, char *error_str){
    fprintf(stderr, "%s: %s : %s\n", PROGRAM_NAME, msg, error_str);
    exit(EXIT_FAILURE);
}