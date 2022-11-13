/**
 * @file mygrep.c
 * @brief Source Code of 'mygrep'
 * @author Philipp Vanek 12022484
 */


#include <stdio.h> 
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

const char *myprog;



/**
 * @brief Prints the programm path, an error message
 *        and a string representation off errno to stderr and exits
 * @param errorMsg string message which should be printed
 */
static void outputErrorAndExit(char* errorMsg) {
    fprintf(stderr, "%s: %s: %s \n", myprog, errorMsg, strerror(errno));
    exit(EXIT_FAILURE);
}



/**
 * @brief Prints the correct usage to stderr and exits
 * @param exitCode Code to exit the program
 */
static void outputUsageAndExit(int exitCode) {
    fprintf(stderr, "Correct usage: %s [-i] [-o outfile] keyword [file...] \n", myprog);
    exit(exitCode);
}




/**
 * @brief handles the given commandline input options with the "getopt" function. 
 *        Afterwards "optind" is left at the first argument.
 * @param argc Amout of given arguments
 * @param argv contains given arguments as strings
 * @param caseSensitivity reference to bool deciding caseSensitivity
 * @return String of outputFile path or null
 */
static char* handleArguments(int argc, char *argv[], bool* caseSensitivity) {
    myprog = argv[0]; // Save the programm name globally
    
    char *outFilePath = NULL;
    int opt_i = 0;
    
    int c;
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i': 
                opt_i++;
                break;
            case 'o':
                outFilePath = optarg;
                break;
            case '?': /* invalid option */
            default:
                outputUsageAndExit(EXIT_FAILURE);
                break;
        }
    }

    if (opt_i > 0) {*caseSensitivity = false;}  // if option -i is given ignore Case Sensitivity
    
    return outFilePath;
}



/**
 * @brief Reads lines from an input file and checks if it contains a given keyword.
 *        If the keyword is contained, print it to the given output file
 * @param outFile File to write sentence containing a keyword
 * @param inFile File where keyword is searched
 * @param keyword to search in each line of given file
 * @param caseSensitivity whether search is case sensitive or not
 * @return negative number on error else 0
 */
static int readLineWithKeyword (FILE* outFile, FILE* inFile, char* keyword, bool caseSensitivity) {
    
    char * line = NULL;
    size_t len = 0;

    // convert keyword to uppercase if case-insensitive
        if(caseSensitivity == false) {
            
            int i;
            for (i = 0; i < strlen(keyword); i++) { 
                keyword[i] = tolower(keyword[i]); 
            } 
        }

    while (getline(&line, &len, inFile) != -1) {
    //while (fgets(&line, &len, inFile) != -1) {

        int lineLength = strlen(line);
        char tempLine[lineLength];


        // convert line to Uppercase if case-insensitive
        if(caseSensitivity == false) {
            strcpy(tempLine, line);
            int i;
            for (i = 0; i < strlen(tempLine); i++) { 
                tempLine[i] = tolower(tempLine[i]); 
            } 
        } 
        else {
            strcpy(tempLine, line);
        }


        if (strstr(tempLine, keyword) != NULL) {
            int a = fprintf(outFile, "%s \n", line);
            if (a < 0) {
                return a;
            }
        }
    }

    return 0;
}


int main(int argc, char *argv[]) {
    
    FILE* outFile;
    bool caseSensitivity = true;

    const char* outFilePath = handleArguments(argc, argv, &caseSensitivity);
    char* keyword = NULL;

    // outputFile handling
    if (outFilePath == NULL) {  // use stdout if no path is given in options
        outFile = stdout;
    } 
    else {
        outFile = fopen(outFilePath, "w");
    }

    if (outFile == NULL) {
        outputErrorAndExit("Couldn't open output file!");
    }



    // keyword handling
    if (optind == argc) {     // No keyword provided
        outputErrorAndExit("No keyword provided!");
    }
    else {
        keyword = argv[optind];
        optind ++;
    }


    // if no input files are specified read from stdin
    if (optind == argc) {
        if (readLineWithKeyword(outFile, stdin, keyword, caseSensitivity) < 0) {
            fclose(outFile);
            outputErrorAndExit("Couldn't write to output file!");
        }
    }

    // inputFile handling
    for (int i = optind; i != argc; i++) { // iterate all inputFiles provided as argument
        FILE* inputFile = fopen(argv[i], "r");

        if (inputFile == NULL) {
            fclose(outFile);
            outputErrorAndExit("Couldn't open specified input file!");
        }

        if (readLineWithKeyword(outFile, inputFile, keyword, caseSensitivity) < 0) {
            fclose(inputFile);
            fclose(outFile);
            outputErrorAndExit("Couldn't write to output file!");
        }
        fclose(inputFile);
    }
    fclose(outFile);

    return EXIT_SUCCESS;
}