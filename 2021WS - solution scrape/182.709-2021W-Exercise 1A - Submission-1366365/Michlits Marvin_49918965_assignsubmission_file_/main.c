/**
 * @file main.c
 * @author Marvin Michlits 11731205
 * @date 04.11.2021
 *
 * @brief Main program module.
 * 
 * This program implements a variation of grep using the functions implemented in grep.c.
 * Various inputs (stdin, input file) and options (-i, -o) are possible and produce different results.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "grep.h"
#include <sys/stat.h>

/**
 * Program entry point.
 * @brief The program starts here. A lot of the implementation of mygrep is in main. 
 * @details The program is started with some arguments in argv. The first thing which is done is main is the checking for options in argv.
 * That is done using getopt. The aceppted options are -i and -o. If -i is in argv the bool variable isCaseSensitive is set to false.
 * If isCaseSensitive is false later on both the grepString and the current line which is read from stdin or the current input file are converted to only capital letters using toupper.
 * If -o is in argv then the program writes to an output file later on. In that case the bool variable writeToStdout is set to false.
 * If it is true the program prints to stdout.
 * After the options are checked the input file(s) or stdin are read and written to either stdout or an output file accordingly.
 * To achieve that the functions implemented in grep.c are used. 
 * global variables: isCaseSensitive, writeToStdout, optionCount, option, outputfile, line, inputfile, filesize, grepString
 * @param argc The argument counter.
 * @param argv The argument vector. This includes the grepString, potential options, potential input files and an output file name if -o is an argument.
 * @return Returns EXIT_SUCESS if it succeeds or EXIT_FAILURE if it fails. 
 */

int main(int argc, char *argv[]) {

    bool isCaseSensitive = true;
    bool writeToStdout = true;
    int optionCount = 0;
    int option;
    FILE *outputFile;

    while ((option = getopt(argc, argv, "oi")) != -1) { /* options in argv are checked using getopt */
        if (option == '?'){ /* checking for getopt error */
            fprintf(stderr, "[%s] ERROR: getopt failed: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        switch (option)
        {
        case 'o':  
            writeToStdout = false; 
            optionCount+=2; /* because output filename is also on the argument vector */
            if((argc - optionCount) == 2) {
                outputFile = openOutput(argv[optionCount], "a+");
            }
            else outputFile = openOutput(argv[optionCount], "w+");
            break;
        case 'i': 
            isCaseSensitive = false; 
            optionCount++; 
            break;
        default: 
            fprintf(stderr, "[%s] ERROR: getopt failed: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
     }
     
    FILE *inputfile;
    char *line; 
    int filesize; 
    char *grepString = argv[1 + optionCount];
    if ((argc - optionCount) == 2) { /* if no file is given the "inputfile" is stdin */
        inputfile = stdin;
        line = malloc(10000); /* not ideal and not unlimited like specified in the task but I don't know how to get the exact input size here */ 
        if (line == NULL) { /* checking for malloc error */
            fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", argv[0], strerror(errno));
            fclose(inputfile);
            free(line);
            exit(EXIT_FAILURE);
        }
        filesize = 100000;
        while((line = fgets(line, filesize, inputfile)) != NULL) { /* gets stdin using fgets */
            int lineSize = strlen(line);
            char originalLine[lineSize];
            strncpy(originalLine, line, lineSize);
            originalLine[lineSize] = '\0'; 
            if (isCaseSensitive == false) {
                for (int j = 0; grepString[j] != '\0'; j++) { /* converts input string to upper case */
                    grepString[j] = toupper(grepString[j]);
                }
                for (int k = 0; line[k] != '\0'; k++){ /* converts line to upper case */
                    line[k] = toupper(line[k]);
                }
                if (writeToStdout == false) {
                    openOutput(argv[optionCount], "a+"); /* open in append mode */
                    writeToOutput(originalLine, line, grepString, outputFile);
                    fclose(outputFile);
                }
                else {
                    writeToStd(originalLine, line, grepString);
                }
            }
            else if (writeToStdout == false) {
                openOutput(argv[optionCount], "a+"); /* open in append mode */
                writeToOutput(originalLine, line, grepString, outputFile);
                fclose(outputFile);
            }
            else { 
                writeToStd(originalLine, line, grepString);
            }
        } 
        free(line);
        fclose(outputFile);
    }
    else { /* unfortunately some duplicate code from above in a loop - could put this into a function but that wouldn't really improve the readability in my opinion as it wouldn't make it much shorter*/
        for (int i = 2 + optionCount; i < argc; i++) { /* opens and reads all input files one after another in the order it is specified in argv */
            inputfile = fopen(argv[i], "r");                       
                struct stat s;
                if (stat(argv[i], &s) != 0) { /* checking for error in stat */
                    fprintf(stderr, "[%s] ERROR: checking of file properties of file %s failed: %s\n", argv[0], argv[i], strerror(errno));
                    exit(EXIT_FAILURE);
                }
                filesize = s.st_size; /* the file size is the maximum possible length for the lines read from the file */
                line = malloc(filesize); /* maximum possible line length is allocated */
                if (line == NULL) { /* checking for malloc errors */
                    fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", argv[0], strerror(errno));
                    fclose(inputfile);
                    free(line);
                    exit(EXIT_FAILURE);
                }
            
                while((line = fgets(line, filesize, inputfile)) != NULL) {  /* gets lines from the current inputfile using fgets */
                    int lineSize = strlen(line); 
                    char originalLine[lineSize];
                    strncpy(originalLine, line, lineSize); /* the current line is copied to originalLine before the current line is potentially changed (if -i is used) */
                    originalLine[lineSize] = '\0'; /* for safety */
                    if (isCaseSensitive == false) {
                        for (int j = 0; grepString[j] != '\0'; j++) { /* converts input string to upper case */
                            grepString[j] = toupper(grepString[j]);
                        }
                        for (int k = 0; line[k] != '\0'; k++){ /* converts line to upper case */
                            line[k] = toupper(line[k]);
                        }
                        if (writeToStdout == false) {
                            writeToOutput(originalLine, line, grepString, outputFile);
                        }
                        else {
                            writeToStd(originalLine, line, grepString);
                        }
                    }
                    else if (writeToStdout == false) {
                        writeToOutput(originalLine, line, grepString, outputFile);
                    }
                    else { 
                        writeToStd(originalLine, line, grepString);
                    }
                }
                if ((writeToStdout == false) && (i < (argc - 1))) {
                    fputs("\n", outputFile); /* new line after 1 file is written if we're writing to output file unless it's the last file which is read */
                }   
                free(line);
                if (writeToStdout == true) {
                    printf("%c",'\n'); /* new line after 1 file is written if we're writing to stdout*/
                }
                fclose(inputfile);
            }
        }
    return EXIT_SUCCESS;
}