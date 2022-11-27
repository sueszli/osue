/**
 * @file   mydiff.c
 * @author Svetlin Tomanov 01567990
 * @date   09.11.2021
 *
 * @brief A program that compares two files line by line.
 *
 * @details The program compares two files line by line and prints out the line number of the lines 
 *          containing differences as well as the number of characters that differ either to stdout or to a specified file.
 * 
 *          If  two  lines  have  different length, then the comparison shall stop upon reaching the end of the shorter line. 
 *          Therefore, the lines abc\n and abcdef\n shall be treated as being identical.
 * 
 *          The program must accept lines of any length.
 * 
 *          If no difference is found, the message "No difference found!" shall be output.
 * 
 *          The program shall receive the paths of the two files to be compared as arguments.
 * 
 *          If the option [-o] is given, the output is written to the specified file (outfile). 
 *          If the file does not exist, it shall be created.
 *          Otherwise, the output is written to stdout.
 * 
 *          If the option [-i] is given, the program shall not differentiate between lower and upper case letters,
 *          i.e. the comparison of the two lines shall be case insensitive.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <strings.h>

#define NUMBER_OF_ARGUMENTS 2 // options can be either [-o] or [-i]

char *programName;

/**
 * @brief usage of mydiff
 * @details Prints the usage of the program to stderr in case of incorrect input.
 */
void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", programName);
    exit(EXIT_FAILURE);
}

/**
 * @brief Parses the program arguments.
 * @details Receives the program arguments from the main function and sets the output parameters.
 *          Calls usage() if any of the compulsory arguments are missing or invalid.
 * 
 * @param argc The argument counter.
 * @param argv The argument values.
 * @param inputFilePath1_out Pointer to a string that contains the path of the first file to be compared 
 *                           after parseArguments terminates.
 * @param inputFilePath2_out Pointer to a string that contains the path of the second file to be compared
 *                           after parseArguments terminates.
 * @param outputFilePath_out Pointer to a string that contains the path of the output file (if [-o] option is invoked) 
 *                           or NULL in default case after parseArguments terminates.
 * @param caseSensitive_out Pointer to an integer that specifies if the program is case sensitive or not
 *                          after parseArguments terminates.
 */
static void parseArguments(int argc, char *argv[], 
                            char **inputFilePath1_out, char **inputFilePath2_out, 
                            char **outputFilePath_out, int *caseSensitive_out)
{
    *caseSensitive_out = 1; // program is case sensitive by default, unless specified otherwise

    // reading program options
    int option = -1;
    while ((option = getopt(argc, argv, "io:")) != -1)
    {
        switch (option)
        {
        case 'i':
            *caseSensitive_out = 0; // make program case insensitive
            break;
        case 'o':
            *outputFilePath_out = optarg; // specify output file path to argument after option [-o]
            break;
        case '?': // any invalid option
            usage();
            break;
        default:
            // no option found leads to an error
            usage();
            break;
        }
    }

    if ((argc - optind) != NUMBER_OF_ARGUMENTS)
    {
        fprintf(stderr, "%s: Invalid number of arguments!\n", programName);
        usage();
    }

    *inputFilePath1_out = argv[optind];
    *inputFilePath2_out = argv[optind + 1];
}

/**
 * @brief Opens the input files and output file (if such is specified).
 * @details In case an output file is specified and has not yet been created (doesn't exist), it shall be created.
 *          Calls usage() if opening of input files (or opening/creating output file when specified) fails.
 * 
 * @param inputFilePath1 Path of the first file to be compared.
 * @param inputFilePath2 Path of the second file to be compared.
 * @param outputFilePath Path of the output file (if [-o] option is invoked) 
 *                       or NULL in default case.
 * @param inputFilePath1_out Pointer to the first file to be opened in read mode 
 *                           after openFiles terminates.
 * @param inputFilePath2_out Pointer to the second file to be opened in read mode
 *                           after openFiles terminates.
 * @param outputFilePath_out Pointer to the output file to be opened in write mode (if [-o] option is invoked) 
 *                           or to stdout if outputFilePath is NULL after openFiles terminates.
 */
static void openFiles(const char *inputFilePath1, const char *inputFilePath2, const char *outputFilePath,
                        FILE **inputFilePath1_out, FILE **inputFilePath2_out, FILE **outputFilePath_out)
{
    *inputFilePath1_out = fopen(inputFilePath1, "r");
    *inputFilePath2_out = fopen(inputFilePath2, "r");

    if(*inputFilePath1_out == NULL || *inputFilePath2_out == NULL)
    {
        fprintf(stderr, "%s: Input files opening failed!\n", programName);
        usage();
    }

    if(outputFilePath != NULL)
    {
        *outputFilePath_out = fopen(outputFilePath, "w");
        if(*outputFilePath_out == NULL)
        {
        fprintf(stderr, "%s: Output file opening/creation failed!\n", programName);
        exit(EXIT_FAILURE);
        }
    } 
    else
    {
        *outputFilePath_out = stdout;
    } 
}

/**
 * @brief Checks if the given parameter is EOF or '\n' (new line).
 * 
 * @param character The character to check.
 */
static inline bool isNewLineOrEof(int character)
{
    if(character == EOF || character == '\n')
    {
        // printf("%s", "Char is new line or EOF\n");
    }
    return character == EOF || character == '\n';
}

/**
 * @brief Checks if any of two given parameters is EOF or '\n' (new line).
 * 
 * @param char1 The first character to check.
 * @param char2 The second character to check.
 */
static inline bool isOneNewLineOrEof(int char1, int char2)
{
    if(isNewLineOrEof(char1) || isNewLineOrEof(char2))
    {
        // printf("%s", "One is new line or EOF\n");
    }
    return isNewLineOrEof(char1) || isNewLineOrEof(char2);
}

/**
 * @brief Takes an opened file, moves the current file position to the next line
 *        and returns the first character of that line.
 *
 * @param file The opened file.
 *
 * @return The first character of the next line.
 */
static int moveToNextLine(FILE *file) 
{
    // get to the end of the current line first
    while(!isNewLineOrEof(fgetc(file)));
    return fgetc(file);
}

/**
 * @brief Compares the given input files and prints the differing lines and number of characters to the output file/stdout.
 * @details The two input files are compared line by one until one of the files ends.
 *          Each pair of lines is compared until one of them reaches its end (i.e. 'abc\n' is equal to 'abcdefg\n').
 *          If the compared pair of lines differ from one another, the line number and number of differing characters is printed.
 *          If the files are deemed equal by the algorithm, "No differences found!" is printed.
 *          If an error occurs while printing out, the program terminates with EXIT_FAILURE.
 * 
 * @param inputFile1 The first file to be compared.
 * @param inputFile2 The second file to be compared.
 * @param outputFile The output file the result is printed to if option [-o] is invoked or stdout otherwise.
 * @param caseSensitive Specifies if the program is case sensitive or not.
 */
static void printDifferences(FILE *inputFile1, FILE *inputFile2, FILE *outputFile, int caseSensitive)
{
    int (*compareChar)(const char*, const char*) = (caseSensitive == 1) ? &strcmp : &strcasecmp;

    int differingLines = 0;

    int currentCharFileInput1 = fgetc(inputFile1);
    int currentCharFileInput2 = fgetc(inputFile2);

    int lineCounter = 1;

    for(; currentCharFileInput1 != EOF && currentCharFileInput2 != EOF; lineCounter++)
    {
        int differencesInLineCounter = 0;
        
        // printf("We are at line: %d\n", lineCounter);

        while (!isOneNewLineOrEof(currentCharFileInput1, currentCharFileInput2) && currentCharFileInput1 != '\r' && currentCharFileInput2 != '\r') // \r check for Windows (NL = \r\n)
        {
            // int result = (*compareChar)((const char *) &currentCharFileInput1, (const char *) &currentCharFileInput2);
            
            // printf("Comparing: %c and %c\n", currentCharFileInput1, currentCharFileInput2);

            // printf("Comparison returns: %d\n", result);

            if ((*compareChar)((const char *) &currentCharFileInput1, (const char *) &currentCharFileInput2) != 0) 
            {
                differencesInLineCounter++;
            }
            
            currentCharFileInput1 = fgetc(inputFile1);
            currentCharFileInput2 = fgetc(inputFile2);

            // printf("\n\nNEW WHILE RUN\n\n");
        }

        if(differencesInLineCounter != 0)
        {
            differingLines++;
            if (fprintf(outputFile, "Line: %i, characters: %i\n", lineCounter, differencesInLineCounter) < 0)
            {
                fprintf(stderr, "%s: Printing difference failed!\n", programName);
                exit(EXIT_FAILURE);
            }
        }

        if(isNewLineOrEof(currentCharFileInput1)) {
            // moves the current file position to the next character, if we have already moved to the next line
            currentCharFileInput1 = fgetc(inputFile1);
        }
        else 
        {
            currentCharFileInput1 = moveToNextLine(inputFile1);
        }
        // printf("Input 1 char is now <%c>\n", currentCharFileInput1);

        if(isNewLineOrEof(currentCharFileInput2)) {
            // moves the current file position to the next character, if we have already moved to the next line
            currentCharFileInput2 = fgetc(inputFile2);
        }
        else 
        {
            currentCharFileInput2 = moveToNextLine(inputFile2);
        }        
        // printf("Input 2 char is now <%c>\n", currentCharFileInput2);


        // printf("\n\nNEW FOR RUN\n\n");
    }

    if(differingLines == 0)
    {
        fprintf(outputFile, "No differences found!");
    }
}

/**
 * @brief Entry point of the program.
 * @details The main function is where the whole program is run.
 * 
 * @param argc The argument counter.
 * @param argv The argument values.
 * 
 * @return EXIT_SUCCESS when successfully run or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[])
{
    // getting programName as specified in argv[0]
    programName = argv[0];

    int caseSensitive = 1; // program is case sensitive by default, unless specified otherwise
    char *inputFilePath1 = NULL;
    char *inputFilePath2 = NULL;
    char *outputFilePath = NULL;

    parseArguments(argc, argv, &inputFilePath1, &inputFilePath2, &outputFilePath, &caseSensitive);

    FILE *inputFile1 = NULL;
    FILE *inputFile2 = NULL;
    FILE *outputFile = NULL;

    openFiles(inputFilePath1, inputFilePath2, outputFilePath, &inputFile1, &inputFile2, &outputFile);

    printDifferences(inputFile1, inputFile2, outputFile, caseSensitive);

    fclose(inputFile1);
    fclose(inputFile2);
    fclose(outputFile);

    // printf("%s", programName);
    return EXIT_SUCCESS;
}