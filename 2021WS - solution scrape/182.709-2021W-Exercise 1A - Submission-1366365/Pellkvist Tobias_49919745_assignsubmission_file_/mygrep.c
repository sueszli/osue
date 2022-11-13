/**
 * @file mygrep.c
 * @author Tobias Pellkvist <e12024024>
 * @date 10.11.2021
 *
 * @brief program which finds input string in input lines
 * 
 * This program takes and input string, an optional output file and 0..n input files
 * in which it writes the lines which contain the input string. if no input/output
 * files are given, stdin/stdout are used. The input option -i makes the program
 * ignore upper/lower case.
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

/*
 * @brief returns the lowercase copy of the input argument
 * @param str The input string
 * @return Returns lowercase copy result
 */
char* toLowerCase(char* str) {
    char* result = calloc(strlen(str), sizeof(char));
    for(int i = 0; str[i]; i++) {
        result[i] = tolower(str[i]);
    }
    return result;
}

/*
 * @brief The program takes an input from either a list of 0..n input files or stdin, 
 * and writes to either stdout or one specified output file, determined by -o option, if the main argument
 * is contained within a given line, either from stdin, or the input files. 
 * 
 * @param argc The argument counter
 * @param argv The argument vector
 * @return Returns EXIT_SUCCESS if program successfully reads and writes
 * @return Returns EXIT_FAILURE if the specified input file does not exist
 */
int main(int argc, char *argv[]) {
    char input[1000];
    FILE* inFile = stdin;
    FILE* outFile = stdout;
    char* keyword = argv[1];
    char* lowerCaseKeyword = NULL;
    char* lowerCaseInput;
    int inFileIndex = 2;
    int c;


    while ((c = getopt (argc, argv, "io:")) != -1)
    switch (c)
      {
      case 'i':
        if (strcmp(argv[optind],"-o") == 0) lowerCaseKeyword = toLowerCase(argv[optind+2]);
        else {
            lowerCaseKeyword = toLowerCase(argv[optind]);
            keyword = argv[optind];
            inFileIndex = optind+1;
        } 
        break;
      case 'o':
        outFile = fopen(optarg, "w");
        keyword = argv[optind];
        inFileIndex = optind+1;
        break;
      case '?':
        break;
      }
  
    if (argv[inFileIndex] != NULL) {
        for (int i = inFileIndex; argv[i]; i++) {
            if (access(argv[i], F_OK) == 0) inFile = fopen(argv[i], "r");
            else {
                fprintf(stderr, "Input file does not exist: %s\n", argv[i]);
                return EXIT_FAILURE;
            }
            while (fgets(input, __INT_MAX__, inFile)) {
                if (lowerCaseKeyword) {
                    lowerCaseInput = toLowerCase(input);
                    if (strstr(lowerCaseInput, lowerCaseKeyword) != NULL) { 
                        fputs(input, outFile);
                    }
                } else if (strstr(input, keyword) != NULL) {
                    fputs(input, outFile);
                }
            }
            fclose(inFile);
        }
    } else {
        fgets(input, __INT_MAX__, stdin);
        
        if (lowerCaseKeyword)  {
            lowerCaseInput = toLowerCase(input);
            if (strstr(lowerCaseInput, lowerCaseKeyword) != NULL) {
                fputs(input, outFile);
            }
        } else if (strstr(input, keyword) != NULL) {
            fputs(input, outFile);
        }
    }
    if (outFile != stdout) {
        fclose(outFile);
    }
    return EXIT_SUCCESS;
}
