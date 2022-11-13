/**
 * @file mygrep.c
 * @author BAUER Jakob Johannes, 12002215
 * @date 14.11.2021
 *
 * @brief Scans files for a specific search term and prints lines that include that term
 *
 * @details Reads lines from a list of files or stdin, and prints the current line to thze specified output if a search tearm occurs in it.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define BUFFERSIZE 4096

/**
 * @brief Prints the usage
 *
 * @details Prints the usage of this program to stderr
 */
static void print_usage(void) {
    fprintf(stderr, "Usage: mygrep [-i] [-o outfile] keyword [file...]");
    exit(EXIT_FAILURE);
}

/**
 * @brief Prints an error
 *
 * @details Prints an error of this program to stderr, in the format [mygrep] ERROR <title>: <details>
 *
 * @param errorTitle The title of the error
 * @param errorMsg The message of the error
 */
static void print_error(char*errorTitle, char*errorMsg) {
    fprintf(stderr, "[mygrep] ERROR %s: %s\n", errorTitle, errorMsg);
    exit(EXIT_FAILURE);
}

/**
 * @brief Converts an string to lowercase
 *
 * @details Converts an string to lowercase
 *
 * @param inp The string that should be converted
 */
static void convertToLower(char*inp) {
    for (int i = 0; inp[i]; i++) {
        inp[i] = tolower(inp[i]);
    }
}

/**
 * @brief Prints lines that contain a keyword from a file
 *
 * @details Scans each line of a file for a specific keyword, and prints the line to outFile if the keyword was found
 *
 * @param inFile The File to scan
 * @param searchKW The keyword to search for
 * @param iFlag A flag to signalize if the scan should be case sesnsitive (0) or insensitiv (1)
 */
static void grepFile(FILE*inFile, char*searchKW, int iFlag, FILE*outFile) {
    char line[BUFFERSIZE];
    char oldLine[BUFFERSIZE];
    // Reads each line from the given file
    while (fgets(line, BUFFERSIZE, inFile) != NULL) {
        // Saves the original line
        strcpy(oldLine, line);
        // If i flag is set, convert all strings to lowercase
        if (iFlag == 1) {
            convertToLower(line);
            convertToLower(searchKW);
        }
        // If the line contains the searchterm, print it
        if (strstr(line, searchKW) != NULL) fprintf(outFile, "%s", oldLine);
    }
}

/**
 * @brief Starts the program and gets needed information from the console
 *
 * @details Starts the program and scans for flags ans parameters. It extracts these parameters and checks for errors.
 * Option -o <outputfile> defines if the output of mygrep should be written to stdout or a file
 * Option -i defines if the search should be case insensitive
 * <keyword> defines the search term that should be searched for
 * A list of filenames defines all files that should be scanned. If it is empty, stdin will be scanned until the program is interrupted
 *
 * @param argc The number of arguments
 * @param argv The arguments
 *
 * @return returns the state of the program when it stops
 */
 int main(int argc, char *argv[])
{
    int iflag = 0;
    FILE*outFile = NULL;
    int c;

    // counts how many arguments are flags
    int argument_counter = 1;

    opterr = 0;
    while ((c = getopt(argc, argv, "io:")) != -1) {
        switch (c) {
            case 'i':
                argument_counter++;
                iflag = 1;
                break;
            case 'o':
                argument_counter += 2;
                outFile = fopen(optarg, "w");
                if (outFile == NULL) print_error("Could not open file", strerror(errno));
                break;
            case '?':
                print_usage();
                break;
            default:
                assert(0);
        }
    }
    if (outFile == NULL) outFile = stdout; // print to console if no file is present

    if (argc == argument_counter) print_usage(); // keyword must be present

    char*search_keyword = argv[argument_counter];

    if (argument_counter + 1 >= argc) { // No files are specified
        grepFile(stdin, search_keyword, iflag, outFile);
    }

    for(int i=argument_counter + 1; i<argc; i++) // go through every inputfile (if specified)
    {
        char* currentFileName = argv[i];
        FILE*currInFile = fopen(currentFileName, "r");
        if (currInFile == NULL) {
            fclose(outFile);
            print_error("Could not open File",  strerror(errno));
        }
        grepFile(currInFile, search_keyword, iflag, outFile);
        fclose(currInFile);
    }

    fclose(outFile);
    return EXIT_SUCCESS;
}