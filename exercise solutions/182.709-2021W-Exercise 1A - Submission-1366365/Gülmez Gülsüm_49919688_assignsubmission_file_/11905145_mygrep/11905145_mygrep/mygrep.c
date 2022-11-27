/**
 * @name mygrep
 * @author Gülsüm Gülmez (11905145)
 * @date   08.11.2021
 *
 * @brief mygrep is a program which goes through several files and prints the lines with the given keyword
 *
 * @details this program has to go through all lines in a file and print those containing the keyword.
 * the lines will be printed in an outfile mentioned in [-o outfile] or in stdout if not. 
 * There is no limit of the length in a line.
 * if the program can't find the inputfile/s then it exits with EXIT_FAILURE
 * [-i] describes the case sensitivity so if case i is true then all it doesn't matter if the letters are capitals or not. 
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

char *mygrep;

void usage(void) {
    fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]", mygrep);
    exit(EXIT_FAILURE);
}

/**
 * @brief handling the program with the options
 * @details open the files, if options i and o are given then write the keyword in the outfile. then end the program with closing the files.
 * @return usage() if there are more options than needed, exit_success if everything goes right, exit_failure if there is no output file
 **/
int main(int argc, char *argv[]) {

    // open the file
    FILE *input = fopen(argv[3], "r");
    if (input == NULL) {
        return EXIT_FAILURE;
    }

    char *output = NULL;
    int i = 0; //sensitive if 0 -> case i = case sensitive
    int o = 0;
    int c;

    // reading the options with getopt
    
    while ( (c = getopt( argc, argv, "io:")) != 1) {

        switch ( c ) {
            case 'i' : i = 1;              
                break;
            case 'o' : output = optarg; o++;
                break;
            case '?' : /* invalid option */
                usage();
                break;
        }   
    }
    
    // number of positional arguments is not 2 (there is no file or keyword)
    if ( (argc - optind) != 2 ) {
        usage();
    }
    // number of options is more than one
    if ( i > 1 || o > 1 ) {
        usage();
    }

    return EXIT_SUCCESS;

    // output file setting
    FILE *outfile;

    if (output == NULL) {
        outfile = stdout;
    } else {
        outfile = fopen(argv[1], "w");
        if (outfile == NULL) {
            return EXIT_FAILURE;
        }
    }

    // closing and setting the pointers on NULL
    fclose(input);
    fclose(outfile);

    input = NULL;
    outfile = NULL;

    return EXIT_SUCCESS;

}

/**
 * @brief reads line by line to print the lines which include the keyword given in the output file
 * @details this program uses fgets and strstr to put out the lines with the keyword 
 * @param keyword
 * @param input which is the inputfile
 * @param outfile which is the output file where the lines with the keyword are printed
 * @return either exit_success or exit_failure in cases the keyword is part of the line and not
 **/

void findKeyword (char *keyword, FILE *input, FILE *outfile) {
    
    char *line = 0;
    int linecounter = 1;
    
    
    if (line != NULL) {

        while (fgets(line, sizeof(line), input) != NULL) {
        
            if(strstr(keyword, line)) {
                fprintf(outfile, line);
                exit( EXIT_SUCCESS );
            } else {
                exit( EXIT_FAILURE );
            }

        linecounter++;
      }
    }

}