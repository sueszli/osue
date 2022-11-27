/**
* @file mygrep.c
* @author David Brand <e11905164@student.tuwien.ac.at>
* @date 01.11.2021
*
* @brief Main program module.
*
* @details Usage: mygrep [-i] [-o outfile] keyword [file...]
* This program reads files line by line and for each line it checks wether it contains
* the search term keyword. The line is printed if it contains keyword, otherwise it is not printed.
* The program accepts lines of any length.
* If one or multiple input files are specified then mygrep reads each of them in the order they are given.
* If no input file is specified, the program reads from stdin.
* 
* If the option -o is given, the output is written to the specified file (outfile).
* Otherwise, the output is written to stdout.
*
* If the option -i is given, the program does not differentiate between lower and upper case letters.
**/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#define BUFFERSIZE 1024

/**
* @brief Searches for a keyword in a string.
*
* @details The method uses method strstr to see if the string keyword
* occurs in the string data.
*
* @param data the string which is to be checked if it contains keyword.
* @param keyword the string which is used to check if it occurs in data.
*
* @return Returns data if it contains keyword, an empty string otherwise.
**/

char *keywordSearch(char *data, char *keyword) 
{
    if (strstr(data, keyword) == NULL) 
    {
        return "";
    }
    return data;
}

/**
* @brief Transforms a string to all lower case letters.
*
* @details The method takes an arbitrary string and converts all letters to lower case.
*
* @param string the string to be converted to lower case.
*
* @return Returns the string in all lower case letters.
**/

char *stringToLower(char *string)
{
    for (int i = 0; i < strlen(string); i++)
    {
        string[i] = tolower(string[i]);
    }
    return string;
}

/**
* @brief Searches for a keyword in a string but case insensitive.
*
* @details The method uses method strstr to see if the string keyword
* occurs in the string data but does not differentiate between lower and upper case letters.
*
* @param dataOriginal the string which is to be checked if it contains keyword.
* @param keyword the string which is used to check if it occurs in data.
*
* @return Returns dataOriginal if it contains keyword, an empty string otherwise.
**/

char *iKeywordSearch(char *dataOriginal, char *keyword) 
{
    char data[strlen(dataOriginal) + 1];
    strcpy(data, dataOriginal);

    stringToLower(data);
    keyword = stringToLower(keyword);
    if (strstr(data, keyword) == NULL)
    {
        return "";
    }
    return dataOriginal;
}

/**
* @brief Reads a line of a file and calls the method to search for a keyword in this line.
*
* @details The method reads line by line from the input file. If the option -i is given,
* the method calls iKeywordSearch to search for keyword in the line case insensitive. Otherwise
* the method calls keywordSearch to search for keyword in the line.
*
* @param input the input file.
* @param output the output file.
* @param keyword the keyword which is passed when calling the program.
* @param opti a flag wether the option -i is passed when calling the program.
**/

void readFile(FILE *input, FILE *output, char *keyword, int opti)
{
        char *line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((read = getline(&line, &len, input)) != -1) 
        {
            if (opti > 0)
            {
                fprintf(output, iKeywordSearch(line, keyword));
            } else
            {
                fprintf(output, keywordSearch(line, keyword));
            }
        }
        free(line);
}

/**
* @brief Main program entry point.
*
* @details The method uses getopt to check which arguments and options are passed
* when the program is called. The method terminates the program when an invalid 
* option or argument is passed. If an output file is specified it opens it.
* If one or more input files are specified the method opens these and calls the method
* readFile. If no input file is passed the method reads from stdin.
* If an output file is passed the result is written in the specified file.
* Otherwise it writes the result to stdout.
*
* @param argc the argument counter.
* @param argv the argument vector.
*
* @return Returns EXIT_SUCCESS or EXIT_FAILURE on error.
**/

int main(int argc, char *argv[]) 
{
    FILE *input;
    FILE *output = stdout;

    char *keyword = NULL;
    int opti = 0;
    int opto = 0;
    char *o_arg = NULL;
    int optn = 0;
    int c;
    while ((c = getopt(argc, argv, "o:i")) != -1)
    {
        switch (c)
        {
        case 'i':
            opti++;
            break;
        case 'o':
            opto++;
            o_arg = optarg;
            break;
        case '?':
            optn++;
            break;
        default:
            break;
        }
    }

    if (optn > 0)
    {
        printf("ERROR: not a valid option \n");
        exit(EXIT_FAILURE);
    }

    if (opto > 0) 
    {
        if (o_arg != NULL) 
        {
            output = fopen(o_arg, "w");
            if (output == NULL)
            {
                fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

    keyword = argv[optind];
    optind++;

    if (optind == argc)
    {
        readFile(stdin, output, keyword, opti);
    }

    while (optind < argc) 
    {
        input = fopen(argv[optind], "r");
        if (input == NULL)
        {
            fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", argv[0], strerror(errno));
            fclose(output);
            exit(EXIT_FAILURE);
        }

        readFile(input, output, keyword, opti);
        fclose(input);
        optind++;
    }

    fclose(output);
    return EXIT_SUCCESS;
}