/** @file ispalindrom.c
* @author Larissa Artlieb 
* @date 25.10.2021
* @brief Implements the functionality required in Uebung 1A
* @details 	This module implements the ispalindrom function */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "ispalindrom.h"
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

//SYNOPSIS:
//        ispalindrom [-s] [-i] [-o outfile] [file...]

//If one or multiple input files are specified (given as positional arguments), then ispalindrom shall read
//each of them in the order they are given. If no input file is specified, the program reads from stdin.

/**
* @brief implements the main funcionality of ispalindrom
* @details implements inputfiles outputfiles, caseSensitivity, ignore whitespaces and is or is not palindrom
* @param argc argument count
* @param argv argument vector
* @return EXIT_SUCCESS on success and EXIT_FAILURE otherwise
*/
int main (int argc, char *argv[]) {

    int opt; 
    int ignoreSpaces = 0; 
    int caseSensitive = 1; 
    int argCount = 1; 
    FILE* outfile = stdout;

    while((opt = getopt(argc, argv, "sio:")) != -1) {
        switch(opt) {
            //The option -s causes the program to ignore whitespaces when checking whether a line is a palindrom
            case 's': 
                ignoreSpaces = 1; 
                argCount++;
                break;
            // If the option -i is given, the program shall not differentiate between lower and upper case letters
            case 'i': 
                caseSensitive = 0; 
                argCount++;
                break; 
            //If the option -o is given, the output is written to the specified file (outfile).
            // Otherwise, the output is written to stdout.
            case 'o':
                argCount += 2;
                outfile = fopen(optarg, "w");

                if(outfile == NULL) {
                    fprintf(stderr, "%s: outfile could not be opened\n", argv[0]);
                    return EXIT_FAILURE;
                }

                break;
            case '?':
                fprintf(stderr, "Usage: %s [-o outfile] [file...] \n", argv[0]);
                return EXIT_FAILURE;
            default: 
            assert(0);
        }
    }
    int fileCount = argc - argCount;
    FILE* inputfiles[fileCount+ 1];
    if(argc > argCount) {
        for(int i = 0; i < (fileCount); i++){
            inputfiles[i] = fopen(argv[argCount + i], "r");
        }
    } else {
        inputfiles[0] = stdin;
        fileCount++;
    }

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    for(int f = 0; f < fileCount; f++) {
        while ((linelen = getline(&line, &linecap, inputfiles[f])) > 0) {
            //overflow includes enter and end of line, which are characters that aren't actually included in the word that's checked
            int overflow = 2;
            char* workingline = line;

            for(int i = 0; i < linelen; i++) {

                if(line[i] != '\n' && line[i] != '\r'){
                    fprintf(outfile, "%c", line[i]);
                }
            }

            if(caseSensitive == 0) {
                for(int i = 0; i <= linelen; i++) {
                    line[i] = tolower((unsigned char)line[i]);
                }
            }

            if(ignoreSpaces == 1) {
                char lineNoSpaces[linelen];
                int j = 0;  

                for(int i = 0; i <= linelen; i++) {
                    if(line[i] != ' ' && line[i] != '\t') {
                        lineNoSpaces[j] = line[i];
                        j++;
                    } else {
                        overflow++;
                    }
                }

                workingline = lineNoSpaces;
            } 

            for (int i = 0; i <= (linelen/2); i++) {

                if(workingline[i] != workingline[linelen-i-overflow]) {
                    fprintf(outfile, " is not a palindrom \n");
                    break;
                } 
                if( i == ((linelen - overflow)/2)) {
                    fprintf(outfile, " is a palindrom \n");
                    break;
                }
            }

            fflush(outfile);
        }
    }

    fclose(outfile);
    return EXIT_SUCCESS; 
}
