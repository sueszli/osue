/**
 * @file main.c
 * @author Paul Pölzl (12022514)
 * @date 27.10.2021
 * 
 * @brief Main program module.
 * 
 * This program reads in several files and prints the lines containing the keyword.
 */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include "source.h"

/** 
 * @brief Program name.
 */
char *myprog;                    

/**
 * @brief Signals the program to shut down.
 * @details Turns to 1 when SIGINT occurs.
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief This function writes helpful usage information to stderr.
 * @details global variables: myprog
 */
static void usage(void) 
{
    fprintf(stderr,"Usage: %s [-i] [-o outfile] keyword [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief A signal handler for SIGINT
 * @details Makes shure the program shuts down correctly after (quit = 1) after receiving SIGINT
 * 
 * @param signal The signal that called the signal handler.
 */
static void handle_signal(int signal)
{
    quit = 1;
}

/**
 * @brief This function writes every line from the input containing the keyword to the output.
 * 
 * @details Expects a line with the format: "mygrep [-i] [-o outfile] keyword [file...]"
 * 
 * If one or multiple input files are specified, then mygrep will read each of them in the order
 * they are given. If no input file is specified the function reads from stdin.
 * 
 * If the option -i is given the program will be case insensitive. If the option -o is given
 * the output is written to the specified outfile. Otherwise the output is written to stdout.
 * 
 * global variables: myprog
 * 
 * @param argc The number of elements in in argv.
 * @param *argv[] Array of command line arguments.
 * 
 * @returns Returns EXIT_SUCCESS on success and EXIT_FAILURE otherwise. 
 */
int main(int argc, char *argv[])
{
    myprog = argv[0];           // Set the program name   

    char *o_arg = NULL;         // The name of the output file 
    int opt_o = 0;              // Option -o (output file) 
    int opt_i = 0;              // Option -i (case sensitivity)
    char *keyword;              // The keyword used to filter the input
    char c;
    
    FILE *in = stdin;           // FILE pointer to the input stream or file 
    FILE *out = stdout;         // FILE pointer to the output stream or file

    struct sigaction sa;            
    memset(&sa, 0, sizeof(sa));     // Initialize sa to 0
    sa.sa_handler = handle_signal;  
    sigaction(SIGINT, &sa, NULL);   // sa reacts to SIGINT

    // Read the selected options and store them in the designated variables
    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
            case 'i':
                opt_i++;
                break;
            case 'o':
                opt_o++;
                o_arg = optarg;
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
                break;
        }
    }

    // Check if the given options -i are valid and if the command contains a keyword
    if (opt_i > 1 || (argc - optind) < 1)              
        usage();

    keyword = argv[optind++];   // Set the keyword

    // If the -o option is used set the output file accordingly
    if (o_arg != NULL)
        out = open(o_arg, "w");

    // If there are no input files given read and filter input from stdin
    if ((argc - optind) == 0) 
        filter(stdin, out, keyword, opt_i);

    // As long as there are input files left open and filter them
    while((argc - optind) > 0 && quit == 0)
    {
        in = open(argv[optind++], "r");
        filter(in, out, keyword, opt_i);
        fclose(in);
    }    
    
    fclose(out);                // Close the output file or stream

    return EXIT_SUCCESS;
}