#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "checkpalindrom.h"
#include "ispalindrom.h"

/**
 * @file ispalindrom.c
 * @author Elias GALL - 12019857
 * @brief Parses arguments and calls the respective functions in 'checkpalindrom.c'
 * @details This module parses and validates arguments and options. Furthermore it includes functions for handling errors.
 * @date 2021-10-29
 */

static void parse_opt_args(int, char*[]);
static void parse_params(int, char*[]);
static void exit_error(const char*, const char*);
static void free_mem(void);
static void handle_external_error(void);

/**
 * The number of times the option 's' was given.
 */ 
static int opt_s = 0;
/**
 * The number of times the option 'i' was given.
 */ 
static int opt_i = 0;
/**
 * The number of times the option 'o' was given.
 */ 
static int opt_o = 0;

/**
 * The name of the output file.
 */ 
char *arg_outfile = NULL;
/**
 * The number of input files.
 */
static int arg_files = 0;
/**
 * The names of the specified input files.
 */
char **arg_infile;
/**
 * The name of the program, set directly in main(). Used for error messages.
 */
char *program_name = NULL;

/**
 * @brief entry point of the program.
 * @details Receives all initial input and calls functions to parse options and arguments,
 *          according to which functions in 'checkpalindrom.c' are called. Global variables
 *          used: program_name
 * @param   argc    number of arguments in argv
 * @param   argv    contains the arguments given by the user  
 * @return  exit code specifying whether the program ran successfully (EXIT_SUCCESS) or not (EXIT_ERROR).
 */
int main(int argc, char *argv[]) {
    program_name = malloc((strlen(argv[0]) + 1) * sizeof(char));
    if (program_name == NULL) {
        exit_error("%s: could not allocate memory", argv[0]);
    }
    strcpy(program_name, argv[0]);
    parse_opt_args(argc, argv);
    parse_params(argc, argv);
   
    if (arg_files == 0) {
        int code = check_std_in_for_palindroms(arg_outfile, opt_s, opt_i);
        if (code != 0) handle_external_error();
    }
    else {
        int i = 0;
        for (i = 0; i < arg_files; i++) {
            int code = check_file_for_palindroms(arg_infile[i], opt_s, opt_i, arg_outfile);
            if (code != 0) handle_external_error();
        }
    }

    free_mem();
    return EXIT_SUCCESS;

}
/**
 * @brief parses options given by the user
 * @details Uses 'getopt' to loop over all options and their parameters and assigns the global 
 *          variables 'opt_s', 'opt_i', 'opt_o' and 'arg_outfile'. Other global variables used:
 *          optarg
 * @param   argc    number of arguments in argv
 * @param   argv    arguments given by the user
 * @return  void
 */
static void parse_opt_args(int argc, char *argv[]) {
    int c = 0;
    while ( (c = getopt(argc, argv, "sio:")) != -1) {
        switch (c) {
            case 's':
                opt_s++;
                break;
            case 'i':
                opt_i++;
                break;
            case 'o':
                opt_o++;
                int len = strlen(optarg);
                arg_outfile = malloc((len + 1) * sizeof(char));
                if (arg_outfile == NULL) {
                    exit_error("%s: could not allocate memory\n", argv[0]);
                }
                strcpy(arg_outfile, optarg);
                arg_outfile[len] = '\0';

                break;
            default:
                exit_error("Usage: %s [-i] [-s] [-o outfile] [file...]\n", argv[0]);
                break;
        }
    }
    // options too often 
    if (opt_i > 1 || opt_s > 1 || opt_o > 1 || (opt_o == 1 && arg_outfile == NULL)) {
        exit_error("Usage: %s [-i] [-s] [-o outfile] [file...]\n", argv[0]);
    }
}
/**
 * @brief   parses program parameters not belonging to options
 * @details Called after 'parse_opt_args', to parse all remaining arguments as file names of input files.
 *          Global variables used: arg_files, arg_infile, optind
 * @param   argc    number of arguments in argv 
 * @param   argv    arguments given by the user
 * @return  void
 */
static void parse_params(int argc, char **argv) {
    char *tmp_arg = NULL;
    int filenames = 0;
    while ((tmp_arg = argv[optind + filenames]) != NULL) {
        filenames++;
    }
    arg_infile = malloc(filenames * sizeof(char *));
    if (arg_infile == NULL) {
        exit_error("%s: could not allocate memory\n", argv[0]);
    }
    int i;
    for (i = 0; i < filenames; i++) {
        arg_infile[i] = malloc(strlen(argv[optind + i]) + sizeof(char));
        if (arg_infile[i] == NULL) {
            exit_error("%s: could not allocate memory\n", argv[0]);
        }
        strcpy(arg_infile[i], argv[i + optind]);
        arg_files++;
    }
    if (opt_o > 0) {
        for (i = 0; i < arg_files; i++) {
            if (strcmp(arg_outfile, arg_infile[i]) == 0) {
                exit_error("%s: output file cannot be input\n", argv[0]);
            }
        }
    }
}
/**
 * @brief   frees memory before exiting the program
 * @details Frees all global char arrays allocated in this module. Global variables used:
 *          arg_infile, arg_files, program_name
 * @param   none
 * @return  void
 */
static void free_mem(void) {
    int i;
    if (arg_infile == NULL) return;
    for (i = 0; i < arg_files; i++) {
        if (arg_infile[i] != NULL) {
            free(arg_infile[i]);
        }
    }
    free(arg_infile);
    free(arg_outfile);
    free(program_name);
}
/**
 * @brief   exits the program when an error occured in this module
 * @details Aborts execution of the program and prints an error message to stderr.
 * @param   msg             message to be printed to stderr, has to include a placeholder for the program name
 * @param   program_name    name of the program, will be included in the error message
 * @return  void
 */
static void exit_error(const char *msg, const char *program_name) {
    fprintf(stderr, msg, program_name);
    free_mem();
    exit(EXIT_FAILURE);
}

void external_error(const char *msg) {
    fprintf(stderr, msg, program_name);
}
/**
 * @brief   closes the program after an error occured outside this module
 * @details Called after an external function returns an error code. Does not print an error message,
 *          since that should be done using 'external_error'
 * @param   none
 * @return  void
 */
static void handle_external_error(void) {
    free_mem();
    exit(EXIT_FAILURE);
}