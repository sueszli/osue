/**
 * @file myexpand.c
 * @author Matteo Kofler <e11904672@student.tuwien.ac.at>
 * @date 30.10.2021
 *
 * @brief Implementation of myexpand function
 * 
 * This program implementa a reduced variation of the Unix-command expand,
 * which reads in several files (or from stdin if no file is given) and
 * replaces all tabulator characters with spaces. 
 * The amount of spaces which the tabulator character is replaced with is calculated
 * via a tabstop distance. 
 * Once the calculation is done, the processed text is either saved to a file 
 * or written to stdout.
 * 
 * For Compilation, the following was used:
 * gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c mygrep.c 
 * To use makefile, type:
 * make all OR make myexpand 
 * To "reset", type:
 * make clean all OR make clean myexpand 
 * 
 * Doxygen help: https://embeddedinventor.com/guide-to-configure-doxygen-to-document-c-source-code-for-beginners/
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>

/** In case of an error, print to stderr and exit program */
/**
 * Error handling.
 * @brief In case of an error in the main method, this function is called.
 * @details
 * An error will be printed to stderr and the program stopped with EXIT_FAILURE
 * @param errMessage The error message to be printed
 * @param argv The name of the program
 * @return void, but program exits with EXIT_FAILURE
 */
void errorAndExit(char *argv, char *errMessage) {
    fprintf(stderr, "%s: %s", argv, errMessage);
    exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief The program starts here. Input is processed and output created here.
 * @details
 * The program reads files line by line and for each line check whether it contains the search term keyword. 
 * The line is printed if it contains keyword, otherwise it is not printed.
 * If one or multiple input files are specified (given as positional arguments after keyword), then they are read in the order they are given. 
 * If no input file is specified, the program reads from stdin.
 * If the option -o is given, the output is written to the specified file (outfile). Otherwise, the output is written to stdout.
 * If the option -i is given, the program shall not differentiate between lower and upper case letters, 
 * i.e. the search for the keyword in a line is case insensitive.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE if the given file couldn't be processed or opened.
 */
int main(int argc, char *argv[]) {
    /** iterator for getopt */
    int option = 0;
    /** Indicates whether text will be printed to a file or stdout */
    bool fileOutput = false;
    /** Indicated whether text will be read from file(s) or stdin */
    bool fileInput = false;
    /** Name of the output file if given by user */
    char outputFileName[100];
    /** tabstop distance, default will be used if nothing given by user */
    int tabstop = 8;
    /** Amount of files given as argument by user */
    int amountOfFiles = 0;
    
    /** Get option parameters */
    while ((option = getopt(argc, argv,"t:o:")) != -1) {
        switch (option) {
            /** tabstop, override default if option is given */
            case 't': ;
                char * tmp;
                int newTabstop = strtol(optarg, &tmp, 10);
                if (newTabstop > 0) {
                    tabstop = newTabstop;
                } else {
                    errorAndExit(argv[0],"Illegal argument -t: (must be a positive integer) \n");
                };
                break;
            /** output file option */
            case 'o': ;
                strcpy(outputFileName, optarg);
                fileOutput = true;
                break;
            /** option without necessary value */
            case ':': ;
                break;
            /** unknown option */
            case '?': ;
                exit(EXIT_FAILURE);
                break;
            default: ;
                assert(0);
                break;
        }
    }

    /** Get file names from positional arguments */
    int index;
    /** get amount of files for array length */
    for (index = optind; index < argc; index++) {
        amountOfFiles++;
    }


    /** save file names to array (malloc -> realloc as alternative) */
    char inputFileNames[amountOfFiles][100];
    int fileNamesIndex = 0;
    index = 0;
    for (index = optind; index < argc; index++) {
        strcpy(inputFileNames[fileNamesIndex], argv[index]);
        fileInput = true;
        fileNamesIndex++;
    }

    /** prepare output file */
    FILE* outputFile;
    if (fileOutput == true) {
        outputFile = fopen(outputFileName, "w");
    }

    /** set 1 in order to access loop below once for stdin input */
    if (fileInput == false) {
        amountOfFiles = 1;
    }

    /** view each file from command line input */ 
    int i;
    int charCount = 0;
    char *finalContent = (char *) malloc(sizeof(char));
    for (i = 0; i < amountOfFiles; i++) {   
        FILE* inputFile;
        /** if a file has been passed, use file ... */
        if (fileInput == true) {
            inputFile = fopen(inputFileNames[i], "r"); 
            /** check if files opens, if not return error with error code: */
            if (inputFile == NULL) {
                char* err = "Error fopen failed \n";
                errorAndExit(argv[0], err);
            } 
        } else { /** ..otherwise use stdin */
            fprintf(stdout, "No input file given. Please provide text content:\n");
            inputFile = stdin;
        }
        /** read line by line: */
        int inLineCount = 0;
        char c; 
        while ( (c = fgetc(inputFile)) != EOF) {
            /** c is tabulator: calc next tabstop "distance", allocate memory and fill out rest of string with spaces */
            if(c == '\t') {
                int p = tabstop * (((inLineCount) / tabstop) + 1);
                int memory = charCount + (p - inLineCount);
                finalContent = (char *) realloc(finalContent, memory * sizeof(char));
                while (inLineCount < p) {
                    finalContent[charCount] = ' ';
                    charCount++;
                    inLineCount++;
                }   
            } else {
               /** regular character: add memory for another character, add character to string */ 
                charCount++;
                finalContent = (char *) realloc(finalContent, charCount * sizeof(char));
                finalContent[charCount-1] = c;
                inLineCount++;
            };
            if (c == '\n') {
                inLineCount = 0;
                /** on enter, stdin reading finishes */
                if (fileInput == false) {
                    break;
                }
            } 
        } 
        if (fileInput) {
            fclose(inputFile);
        } 
        /** before the next file is processed, add a new line (not after last file) */
        if ((i+1) < amountOfFiles) {
            charCount++;
            finalContent = (char *) realloc(finalContent, charCount * sizeof(char));
            finalContent[charCount-1] = '\n';
            inLineCount = 0;
        }

    }
    /** write the processed text to a file or stdout depending on flag set or not */
    if (fileOutput == true) {
        fprintf(outputFile, "%s", finalContent);
        fclose(outputFile);
    } else {
        fprintf(stdout, "%s\n", finalContent);
    } 
    exit(EXIT_SUCCESS);
}
