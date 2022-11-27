/**
 * @file main.c
 * @author David Jahn, 12020634
 * @brief The myGrep program
 * @version 1.0
 * @date 2021-11-13
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/**
 * @brief The name of the program
 * 
 */
const char *PROGRAM_NAME;

/**
 * @brief Method used to print an error message to the screen and terminate the programm
 * 
 * @param error Error message to be printed on the screen
 */
static void error(char *error)
{
    fprintf(stderr, "Error in %s : ", PROGRAM_NAME);
    fprintf(stderr, error); 
    fprintf(stderr, "%s", strerror(errno));
    fprintf(stderr, "\n");
    fflush(stderr);
    exit(EXIT_FAILURE);
}

/**
 * @brief Method to print how to use myGrep and terminate the program
 * 
 */
static void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-o outputfile] keyword [inputfiles]\n", PROGRAM_NAME);
    exit(EXIT_FAILURE);
}
/**
 * @brief A method to write a char* to a file
 * 
 * @param lineToWrite The line to write in the specified file
 * @param filename The name of the file to write to
 */
static void writeToFile(char *lineToWrite, char *filename)
{
    FILE *file = fopen(filename, "a");
    if (file == NULL)
    {
        error("fopen failed: ");
    }
    if (fputs(lineToWrite, file) == EOF)
    {
        error("fputs failed");
    }
}

/**
 * @brief The myGrep function using input files
 * 
 * @param inputFilesLen Describes the length of the arrays of used input files
 * @param inputFiles The files where the input comes from
 * @param keyword The keyword to look for
 * @param caseInsensitive Wether the search after the keyword is case sensitive or not
 * @param useOutputFile Wether to print output to console or to a specific file
 * @param outFile The file to write the output to
 */
static void grepUseInputFiles(int inputFilesLen, char *inputFiles[], char *keyword, int caseInsensitive, int useOutputFile, char *outFile)
{
    FILE *file;
    char line[1024];

    for (int i = 0; i < inputFilesLen; i++)
    {
        file = fopen(inputFiles[i], "r");
        if (file == NULL)
        {
            //error occured
            error("error while fopen");
        }

        while ((fgets(line, sizeof(line), file)) != NULL)
        {
            char *ret = NULL;
            if (caseInsensitive)
            {
                ret = strcasestr(line, keyword);
            }
            else
            {
                ret = strstr(line, keyword);
            }

            if (ret != NULL)
            {
                //Found
                if (useOutputFile)
                {
                    writeToFile(line, outFile);
                }
                else
                {

                    fprintf(stdout, line);
                    fflush(stdout);
                }
            }
        }
        if (fclose(file) != 0)
        {
            error("error while closing file");
        }
    }
}

/**
 * @brief The myGrep function using input from console
 * 
 * @param keyword The keyword to look for
 * @param caseInsensitive Wether the search after the keyword is case sensitive or not
 * @param useOutputFile Wether to print output to console or to a specific file
 * @param outFile The file to write the output to
 */
static void myGrepFromConsole(char *keyword, int caseInsensitive, int useOutputFile, char *outFile)
{

    char line[1024];
    if (fgets(line, sizeof(line), stdin) == NULL)
    {
        //error occured
        error("error while fgets");
    }
    if (ferror(stdin))
    {
        //fgets failed
        error("fgets failed bitch");
    }

    char *ret = NULL;
    if (caseInsensitive)
    {
        ret = strcasestr(line, keyword);
    }
    else
    {
        ret = strstr(line, keyword);
    }
    if (ret != NULL)
    {
        //Found
        if (useOutputFile)
        {
            writeToFile(line, outFile);
        }
        else
        {
            fprintf(stdout, line);
            fflush(stdout);
        }
    }
}

/**
 * @brief Main Function
 * 
 * @param argc Argument counter
 * @param argv Array of arugments
 * @return int 
 */
int main(int argc, char *argv[])
{

    PROGRAM_NAME = argv[0];
    char *keyword = NULL;
    int iOption = 0;
    int oOption = 0;
    char *outfile = NULL;
    char *inputFiles[20];
    int useInputFiles = 0;

    opterr = 0;
    int tmp;

    while ((tmp = getopt(argc, argv, "io:")) != -1)
    {
        switch (tmp)
        {
        case 'i':
            iOption = 1;
            break;
        case 'o':
            oOption = 1;
            outfile = optarg;
            break;
        default:
            usage();
            break;
        }
    }
    if ((argc - optind) < 1)
    {
        usage();
    }
    keyword = argv[optind];

    int inputFilesLength = 0;
    optind++;

    for (int i = optind; i < argc; i++)
    {
        useInputFiles = 1;
        inputFiles[inputFilesLength] = argv[i];
        inputFilesLength++;
    }
    // fprintf(stdout, "keyword: %s, ioption %d, oOpt: %d, useInputFiles: %d, inputfile[0]: %s, Outfile: %s\n", keyword, iOption, oOption, useInputFiles, inputFiles[0], outfile);
    // fflush(stdout);

    if (useInputFiles)
    {
        grepUseInputFiles(inputFilesLength, inputFiles, keyword, iOption, oOption, outfile);
        fprintf(stdout, "\n");
    }
    else
    {
        for (;;)
        {
            myGrepFromConsole(keyword, iOption, oOption, outfile);
        }
    }
    return EXIT_SUCCESS;
}
