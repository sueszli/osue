/**
 * @file ispalindrom.c
 * @author Stefan Hettinger <e11909446@student.tuwien.ac.at>
 * @date 23.10.2021
 *
 * @brief Main program module.
 * 
 * @details This program implements the first assignment (A) called "ispalindrom".
 * Various inputs can be taken, which then get checked for the palindrom property (ABA, ABBA, BBABB, ...).
 * The output is either printed to stdout or in a provided outFile.
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

//global variables
static const char *PROGNAME = "undefined";

//List of functions:
static void fileHandler(FILE *outputFile, FILE *inputFile, bool ignoreWhitespace, bool ignoreCase);
static void usage(char *message);
static void exitFailure(char *message);

/**
 * Program entry point.
 * @brief The main function parses the input arguments, then calls fileHandler()
 * to scan all words and check for palindroms in the provided files.
 * @details This function firstly checks the inputs and exits if not correct.
 * It then sets flags (ignore_whitespace and ignore_case) to pass on to the fileHandler().
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS (0) or EXIT_FAILURE (1)
 */
int main(int argc, char *argv[]) {
    PROGNAME = argv[0];
    int opt;                            //placeholder for the input parameters
    bool ignore_whitespace = false, ignore_case = false;  //flags for option [-s] and [-i]
    FILE *outputPath = stdout;          //default output to stdout, override with [-o outfile]

    //parse Arguments
    while((opt=getopt(argc, argv, "sio:")) != -1) {
        switch (opt) {
            case 's':
                ignore_whitespace = true;
            break;

            case 'i':
                ignore_case = true;
            break;

            case 'o':
                outputPath = fopen(optarg, "w");

                //check output file
                if(outputPath == NULL) {
                    usage("Output file could not be opened!");
                }
            break;
        
            case '?':
                usage("Unknown/Wrong argument found!");
            break;

            default:
                //assert(0); //removed because of compiler
                usage("Error while parsing arguments! (how did you get here?)");
            break;
        }
    }

    //check input type
    if(optind == argc) { //no input files defined
        fileHandler(outputPath, stdin, ignore_whitespace, ignore_case);
        fclose(stdin);
        fclose(outputPath);
    } else { //one or more input files found

        int i;
        for(i = optind; i < argc; i++) { //loop over input files

            if(argv[i] == NULL) //probably not reachable
                usage("Error reading input file!");

            FILE *inputFile = fopen(argv[i], "r");

            if(inputFile == NULL) {
                fprintf(stderr, "%s ERROR: \"Input file \'%s\' could not be opened!\"\n", PROGNAME, argv[i]);
                fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", PROGNAME);
                continue;
            }

            fileHandler(outputPath, inputFile, ignore_whitespace, ignore_case);
            fclose(inputFile);
        }
        fclose(outputPath);
    }
    exit(EXIT_SUCCESS);
}

/**
 * @brief The fileHandler() function scans the inputFile for words, then does a
 * palindrom check.
 * @details This function firstly checks the inputs and exits if not correct.
 * It then removes newline characters, removes whitespaces if flag is set and
 * ignores case sensitivity if flag is set, all line by line.
 * The palindrom check takes the first and last character, does a comparison
 * and moves in by one character. If chars are unequal, the isPalindrom flag
 * is set to false and the next line gets read.
 * @param outputFile Either stdout or the provided File.
 * @param inputFile Either stdin or the provided File.
 * @param ignoreWhitespace Boolean flag to ignore all whitespaces.
 * @param ignoreCase Boolean flag to ignore case sensitivity.
 * @return Returns void
 */
static void fileHandler(FILE *outputFile, FILE *inputFile, bool ignoreWhitespace, bool ignoreCase) {
    char *line = NULL;  //for getline()
    size_t len = 0;     //for getline()
    ssize_t nread;      //for getline()

    while((nread = getline(&line, &len, inputFile)) != -1) { //read over lines of inputFile

        char sanitizedLine[strlen(line)+1]; //to hold line or line without '\n'
        bool isPalindrom = true;

        if(line[nread-1] == '\n') //check for '\n' at the EOL
            line[nread-1] = '\0';

        if(ignoreWhitespace) {

            int i = 0, j = 0;
            while(line[i] != '\0') { //do until EOL
                if(line[i] != ' ')   //skip over whitespace
                    sanitizedLine[j++] = line[i];

                i++;
            }
            sanitizedLine[j] = '\0';
        } else {
            strncpy(sanitizedLine, line, strlen(line)+1);
        }

        int i, j;
        for(i=0, j=strlen(sanitizedLine)-1; i <= j; i++, j--) { //check single line: i is cursor on first char, j is cursor on last char. (i-> <-j)

            if(ignoreCase) {
                if(tolower(sanitizedLine[i]) != tolower(sanitizedLine[j])) {
                    isPalindrom = false;
                    break;
                }
            } else {
                if(sanitizedLine[i] != sanitizedLine[j]) {
                    isPalindrom = false;
                    break;
                }
            }
        }

        if(isPalindrom) {
            fprintf(outputFile, "%s is a palindrom\n", line);
        } else {
            fprintf(outputFile, "%s is not a palindrom\n", line);
        }
    }
    free(line);

    //check for getline errors
    if(errno == EINVAL)
        exitFailure("Bad arguments while using getline! (errno: EINVAL)");
    
    if(errno == ENOMEM) //not sure if even reachable in this version
        exitFailure("Alloc. or realloc. of line buffer failed! (errno: ENOMEM)");
}

/**
 * @brief The function usage is used to exit the program and write a provided
 * error message and the standard usage command to stderr.
 * @details This function is called whenever unexpected behavor is detected
 * and faulty input is expected to have caused this error.
 * @param msg The provided error message.
 * @return Returns void, calls EXIT_FAILURE.
 */
static void usage(char *msg) {
    fprintf(stderr, "%s ERROR: \"%s\"\n", PROGNAME, msg);
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", PROGNAME);
    exit(EXIT_FAILURE);
}

/**
 * @brief The function exitFailure is used to exit the program and write a provided
 * error message to stderr.
 * @details This function is called whenever unexpected behavor is detected. The
 * error message has to be provided manually.
 * @param msg The provided error message.
 * @return Returns void, calls EXIT_FAILURE.
 */
static void exitFailure(char *msg) {
    fprintf(stderr, "%s ERROR: \"%s\"\n", PROGNAME, msg);
    exit(EXIT_FAILURE);
}



