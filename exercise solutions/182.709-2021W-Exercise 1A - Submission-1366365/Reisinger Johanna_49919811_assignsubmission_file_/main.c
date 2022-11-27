/**
 * @file main.c
 * @author Johanna Reisinger
 * @date 11/2021
 * 
 * @brief Main program module used for finding palidromes.
 * 
 * @details Program that takes zero, one or multiple files containing Strings an checks if they
 *          are palindromes or not. If output file is specified, writes result to output file.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define TRUE 1
#define FALSE 0

char *programName = NULL;
FILE *outputFp = NULL;
FILE *inputFp = NULL;

/**
 * @brief tries to free all used resources
 * 
 * 
 */
void clearResources(void)
{
    if (inputFp)
    {
        fclose(inputFp);
        inputFp = NULL;
    }
    if (outputFp)
    {
        fclose(outputFp);
        outputFp = NULL;
    }
}

/**
 * @brief returns true if String is palindrome, alse false
 * 
 * @param str to be checked
 * @param iFlag if 1 then check is not case sensitive, if 0 check is case sensitive
 * @param sFlag if 1 whitespaces are ignored, if 0 whitespaces are to considered
 * @return int 1 on success 0 on failure
 */
int isPalindrome(char *str, int iFlag, int sFlag)
{
    // Start from leftmost and rightmost corners of str
    int l = 0;
    int h = strlen(str) - 1;
    char lchar;
    char hchar;

    // Keep comparing characters while they are same
    while (h > l)
    {
        lchar = str[l];
        hchar = str[h];

        while (sFlag && lchar == ' ' && h > l) //ignores whitespaces
        {
            lchar = str[++l];
        }
        while (sFlag && hchar == ' ' && h > l) //ignores whitespaces
        {
            hchar = str[--h];
        }

        if (iFlag)
        {
            lchar = tolower(lchar);
            hchar = tolower(hchar);
        }

        if (lchar != hchar)
        {
            return FALSE;
        }
        l++;
        h--;
    }
    return TRUE;
}

/**
 * @brief he usage function - shall be used if the program call was faulty
 */
void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-s] [-o OUTFILE] [FILE...]\n", programName);
    clearResources();
    exit(EXIT_FAILURE);
}

/**
 * @brief checks line by line if input from file is palidrome
 * 
 * @param inputFp file to get input from
 * @param outputFp file to write output to
 * @param sFlag 1 if whitespaces shall be ignored, else 0
 * @param iFlag 1 if caseinsensitive, else 0
 */
void parsePalindrom(FILE *inputFp, FILE *outputFp, int sFlag, int iFlag)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, inputFp)) != -1)
    {
        // trim linebreak
        line[strcspn(line, "\r\n")] = '\0';
        if (!strlen(line) == 0)
        {
            const char *msg = isPalindrome(line, iFlag, sFlag) ? "is a palindrom" : "is not a palindrom";
            fprintf(outputFp, "%s %s\n", line, msg);
        }
    }

    if (line)
    {
        free(line);
        line = NULL;
    }
}

/**
 * @brief takes options -i -s -o and files. Takes input for files if provided, else takes from stdin
 *        Calls helper functions and writed output to output file if provided, to stdout else
 * 
 * @param argc argument count
 * @param argv arguemnt values expected to contain the edges
 * @return int program status
 */
int main(int argc, char **argv)
{
    programName = argv[0];
    char *outputPath = NULL;
    char *inputPath = NULL;
    int sFlag = 0;
    int iFlag = 0;
    int oFlag = 0;
    int c = 0;

    while ((c = getopt(argc, argv, "o:si")) != -1)
        switch (c)
        {
        case 'o':
            if (oFlag)
            {
                usage();
            }
            oFlag = 1;
            outputPath = optarg;
            break;
        case 's':
            if (sFlag)
            {
                usage();
            }
            sFlag = 1;
            break;
        case 'i':
            if (iFlag)
            {
                usage();
            }
            iFlag = 1;
            break;
        case '?':
            usage();
            break;
        default:
            assert(0);
        }

    //output handling
    if (oFlag == 0)
    {
        outputFp = fdopen(STDOUT_FILENO, "w");
        if (outputFp == NULL)
        {
            fprintf(stderr, "Error: Could not open standard output");
            usage();
        }
    }
    else
    {
        outputFp = fopen(outputPath, "w");
        if (outputFp == NULL)
        {
            fprintf(stderr, "Error: Could not open file %s\n", outputPath);
            usage();
        }
    }

    //input handling
    if (optind == argc)
    {
        inputFp = fdopen(STDIN_FILENO, "r");
        if (inputFp == NULL)
        {
            fprintf(stderr, "Error: Could not open standard input");
            usage();
        }
        parsePalindrom(inputFp, outputFp, sFlag, iFlag);
    }
    else
    {
        for (int i = optind; i < argc; i++)
        {
            inputPath = argv[i];
            inputFp = fopen(inputPath, "r");
            if (inputFp == NULL)
            {
                fprintf(stderr, "Error: Could not open file %s\n", inputPath);
                usage();
            }
            parsePalindrom(inputFp, outputFp, sFlag, iFlag);
            fclose(inputFp);
            inputFp = NULL;
        }
    }

    clearResources();
    exit(EXIT_SUCCESS);
}