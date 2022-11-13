/**
 * @file ispalindrom.c
 * @author Sejdefa Ibisevic <e11913116@student.tuwien.ac.at>
 * @brief Program that determines if given phrase is palindrom.
 * @details The program takes an input from input file or stdin, checks if input lines are palindrom and
 * writes the answer to output file or stdout
 * @version 0.1
 * @date 2021-10-24
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#define PROGRAM_NAME "ispalindrom"

FILE *outFile;            //output File pointer
FILE *inFile;             //intput file pointer
char *outFileName = NULL; //output file name pointer
int ignoreWhitespace = 0; //flag for -s argumet
int caseInsensitive = 0;  //flag for -i argument
int toOutputFile = 0;     // flag for -o argument
int fromInputFile = 0;    //flag for input file argument

/**
 * @brief write error message and EXIT_FAILURE
 * @param error_message message to be displayed as an error
 * @return void
 */

static void error(char *error_message);

/**
 * @brief function tprovides correct argument input
 * @return void
 */

static void usage(void);

/**
 * @brief Reads input line from file or stdin and calls the function palindromCheck to determine if line is palindrom
 * @details The function uses getline help function to take a line from input. The new-line character is replaced
 * by null-character at the end and palindromCheck is called upon the line. This is repeated for every line on iput.
 * Afterwards, the resources are freed.
 * @param in input file or stdin to be read
 * @return void
 */

static void readInput(FILE *in);

/**
 * palindromCheck function, main program function
 * @brief checks if string is palindrom or not, writes result to an output file or stdout
 * @details The function input argument is character array str which needs to be checked for palindrome existance.
 * If flags for ignoreWhitespaces and caseInsensitive are set, the input is chaged accordingly using help functions
 * rmvWhitespaces and tolower(3). If toOutputFile flag is set, the result will be written to a file with filename
 * outputFilename. Otherwise the result is printed on stdout. The palindrom algorithm compares mirroring characters
 * from left and right side of input line. If pair of character is not equal, the results writes that line is not a
 * palindrom. Otherwise, it is written to output that line is palindrom.
 * @param str character array for palindrom check
 * @return void
 */

static void palindromCheck(char str[]);

/**
 * rmvWhitespaces function, help function
 * @brief Function removes any whitespaces in string str
 * @details Function takes a character pointer str. It uses counter for non-whitespace characters.
 * If character is not a whitespace, it is placed on an index count (starts from 0), and count is incremented.
 * In case of Whitespace, the character is jumped and on its place comes next non-whitespace character.
 * In this way all whitespaces are moved to the end of string. Finally, after the last non-whitespace character
 * null-character is set. Therefore, all whitespaces at the end are cut out.
 * @param str character pointer to remove whitespaces from
 * @return void
 */

static void rmvWhitepaces(char *str);

int main(int argc, char *argv[])
{
    int opt_count = 0;
    char c;

    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        case 's':
            ignoreWhitespace = 1;
            opt_count++;
            break;
        case 'i':
            caseInsensitive = 1;
            opt_count++;
            break;
        case 'o':
            toOutputFile = 1;
            opt_count += 2;
            if (argv[opt_count] != NULL)
            {
                outFileName = argv[opt_count];
            }
            break;
        default: //all other options
            usage();
            break;
        }
    }

    if (toOutputFile)
    {
        outFile = fopen(outFileName, "w");
        if (outFile == NULL)
        {
            error("Failed to open output file");
        }
    }
    //case if input file is provided
    if (argc - 1 > opt_count)
    {
        for (size_t i = opt_count + 1; i < argc; i++)
        {
            inFile = fopen(argv[i], "r");

            if (inFile == NULL)
            {
                error("Failed to open input file");
            }

            fromInputFile = 1;

            //read from file
            readInput(inFile);
        }
    }
    else
    {
        //fflush(stdin);
        if(toOutputFile) fclose(outFile);
        readInput(stdin);
    }
    /*
    if (fromInputFile)
        fclose(inFile);
    if (toOutputFile)
        fclose(outFile);
*/
    return EXIT_SUCCESS;
}

static void error(char *error_message)
{
    fprintf(stderr, "[%s] ERROR: %s: %s.\n", PROGRAM_NAME, error_message, strcmp(strerror(errno), "Success") == 0 ? "Failure" : strerror(errno));
    if (toOutputFile)
        fclose(outFile);
    if (fromInputFile)
        fclose(inFile);
    exit(EXIT_FAILURE);
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}

static void readInput(FILE *in)
{

    char *line = NULL;
    size_t length = 0;
    ssize_t res;
    while ((res = getline(&line, &length, in)) != -1)
    {
        if (line[res - 1] == '\n')
        {
            line[res - 1] = '\0';
            --res;
        }
        if(fromInputFile == 0 && toOutputFile == 1) {
            outFile = fopen(outFileName, "a");
            if (outFile == NULL)
            {
                error("Failed to open output file");
            }
        }
        palindromCheck(line);
        if(fromInputFile == 0 && toOutputFile == 1){
            fclose(outFile);
        }

        //line = NULL;
        //fflush(stdin);
    }

    free(line);

    if (fromInputFile)
        fclose(inFile);
    if (toOutputFile && fromInputFile)
        fclose(outFile);
}

static void palindromCheck(char line[])
{

    int len = strlen(line);
    char temp[len];
    strcpy(temp, line);

    if (caseInsensitive)
    {
        //all Upper case to Lower case
        for (int i = 0; i < len; i++)
        {
            line[i] = tolower(line[i]);
        }
    }

    if (ignoreWhitespace)
    {
        //moving all whitespace to the end and cutting it with '\0'
        rmvWhitepaces(line);
    }

    len = strlen(line);
    int l = 0;
    int r = strlen(line) - 1;

    int notPalindrom = 0;
    if (toOutputFile)
    {
        while (r > l)
        {
            if (line[l++] != line[r--])
            {
                notPalindrom = 1;
                strcat(temp, " is not a palindrom\n");
                if (fputs(temp, outFile) == EOF)
                {
                    error("Writing to output file failed");
                }
                return;
            }
        }
        if (notPalindrom == 0)
        {
            strcat(temp, " is a palindrom\n");
            if (fputs(temp, outFile) == EOF)
            {
                error("Writing to output file failed");
            }
        }
    }
    else if (!toOutputFile)
    {
        outFile = stdout;
        while (r > l)
        {
            if (line[l++] != line[r--])
            {
                notPalindrom = 1;
                strcat(temp, " is not a palindrom\n");
                if (fputs(temp, outFile) == EOF)
                {
                    error("Writing to stdout failed");
                }
                return;
            }
        }
        if (notPalindrom == 0)
        {
            strcat(temp, " is a palindrom\n");
            if (fputs(temp, outFile) == EOF)
            {
                error("Writing to stdout failed");
            }
        }
    }
}

static void rmvWhitepaces(char *line)
{
    int count = 0;

    for (int i = 0; line[i]; i++)
    {
        if (line[i] != ' ')
        {
            line[count++] = line[i];
        }
    }
    line[count] = '\0';
}
