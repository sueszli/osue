/**
 * @file mydiff.c
 * @author Manuel Märzinger <e12021350@student.tuwien.ac.at>
 * @date 12.11.2021
 * 
 * @brief Main program module
 * 
 * This programm compares two text files linewise and prints the amount of differing characters to
 * either stdout or a specified output file when using option -o
 * option -i makes the comparison ignore case sensitivity
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

static void checkEquality(FILE *file1, FILE *file2, int caseSensitive, FILE *output);
static int isEqual(char ch1, char ch2, int caseSensitive);
static void handleLineEnd(FILE *output, FILE *catchUpFile, char *catchupChar, char *aheadChar, char *backN, int *line, int *currLineError);

/**
 * Program entry point.
 * @brief This function handels the option input aswell as the textfiles that get passed in the program call
 * and initiates the equality check of the given textfiles
 * @param argc the argument counter
 * @param argv the argument vector
 * @return returns either EXIT_SUCCESS on program success or EXIT_FAILURE on program failure
 **/
int main(int argc, char *argv[])
{
    FILE *file1;
    FILE *file2;

    int caseSensitive = 0;      // default is 0 => comparison is case sensitive
    char *outputFile = NULL;    // output file => default is stdout

    /* option handling*/
    int c;
    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            caseSensitive = 1;
            break;
        case 'o':
            outputFile = optarg;
            break;
        case '?': /* invalid option */
            if(c == 'o'){
                fprintf(stderr, "The option '-o' requires an argument!\nSource: %s\n", argv[0]);
            } else{
                fprintf(stderr, "Unknown command, use '-i' or '-o arg' instead\nSource: %s\n", argv[0]);
            }
            return EXIT_FAILURE;
            break;
        default:
            assert(0);
        }
    }

    /* checks if there are two input files to read from*/
    if ((argc - optind) != 2)
    {
        fprintf(stderr, "Missing input arguments!\nSource: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* file paths to comparison files */
    char *path1 = argv[optind];         
    char *path2 = argv[optind + 1];

    /* open files in read mode*/
    file1 = fopen(path1, "r");
    file2 = fopen(path2, "r");

    /* in case either file does not exist or any problem occures when opening the file print an error message to stderr */
    if (file1 == NULL || file2 == NULL)
    {
        fprintf(stderr, "Could not open file! \nPlease check whether the file exists or you have reading rights!\nSource: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* open outputfile in write mode if option -o is used*/
    FILE *output = NULL;
    if(outputFile != NULL){
         output = fopen(outputFile, "w");
    }

    checkEquality(file1, file2, caseSensitive, output);

    fclose(file1);
    fclose(file2);
    if(output != NULL){
        fclose(output);
    }
    

    return EXIT_SUCCESS;
}

/**
 * Checks equality of two files line by line.
 * @brief Check equality of two files line by line. If the lines differ in length stop comparison of said line, thus 
 * abc and abcdf will be considered as equal. If characters differ increment a counter. On end of line print 
 * current line and said counter if counter is greater than 0. Comparison stops upond reaching EOF on either file.
 * @details this function works for lines of each length and has a param for case sensitivity. Order in which files are passed does not matter.
 * @param file1 File which has to be compared
 * @param file2 File which file1 has to be compared to 
 * @param caseSensitive indicates whether the comparison should be case sensitive or not. 0 is case sensitive, != 0 is NOT case sensitive
 * @param output the outputfile to which the output should be written. NULL if output should be written to stdout instead
**/
static void checkEquality(FILE *file1, FILE *file2, int caseSensitive, FILE *output)
{
    char chFile1, chFile2;
    char backN = '\n';
    int line = 1, currLineError = 0;

    // go through both files line wise
    do
    {
        // get next char in file
        chFile1 = fgetc(file1);
        chFile2 = fgetc(file2);

        // check if either line ended or not
        if (isEqual(chFile1, backN, caseSensitive) == 0)
        {
            /* file 1 is ahead and file 2 needs to catch up*/
            handleLineEnd(output, file2, &chFile2, &chFile1, &backN, &line, &currLineError);
        }
        else if (isEqual(chFile2, backN, caseSensitive) == 0)
        {
            /* file 2 is ahead and file 1 needs to catch up*/
            handleLineEnd(output, file1, &chFile1, &chFile2, &backN, &line, &currLineError);
        }

        /* both chars are valid options => check for equality */
        if (isEqual(chFile1, chFile2, caseSensitive) != 0)
        {
            currLineError += 1;
        }
    } while (chFile1 != EOF && chFile2 != EOF);
}

/**
 * Checks equality of two characters.
 * @brief checks if two characters are euqal or not.
 * @details option for case sensitivity available.
 * @param ch1 character which has to be compared
 * @param ch2 character which ch1 has to be compared to
 * @param caseSensitive indicates whether the comparison should be case sensitive or not. 0 is case sensitive, != 0 is NOT case sensitive
 * @return returns 0 if characters are equal. returns < 0 if ch1 is less than ch2 and > 0 if ch2 is less than ch1
 **/
static int isEqual(char ch1, char ch2, int caseSensitive)
{
    if (caseSensitive == 0)
    {
        /* case sensitive comparison*/
        return strncmp(&ch1, &ch2, 1);
    }
    else
    {
        /* case insensitive comparison*/
        return strncasecmp(&ch1, &ch2, 1);
    }
}

/**
 * Handling of the end of a line aswell as output. 
 * @brief this method handles the end of a line. Upond a line reaching the end print the result of the comparison, mainly the current line
 * and the number of inequalities in said line. if the lines differ in length, make the longer line catch up to the line which already reached \\n
 * @details aheadChar has to be either \\n or EOF or the method will not work as intended 
 * @param output the outputfile to which the output should be written. NULL if output should be written to stdout instead
 * @param catchUpFile the file of which line has to catch up
 * @param catchUpChar the current character of the catchUpFile
 * @param aheadChar the current char of the file which line is ahead, either \\n or EOF
 * @param backN the char \\n
 * @param line number of current line
 * @param currLineError the current amount of mismatches in current line
**/
static void handleLineEnd(FILE *output, FILE *catchUpFile, char *catchUpChar, char *aheadChar, char *backN, int *line, int *currLineError){
    if (*currLineError != 0)
            {
                /* writes either to outputfile or stdout depending on if -o was given or not*/
                if (output != NULL)
                {
                    fprintf(output, "Line: %d, characters: %d \n", *line, *currLineError);
                }
                else
                {
                    printf("Line: %d, characters: %d \n", *line, *currLineError);
                }
            }

            /* go to next line and reset the inequality counter */
            (*currLineError) = 0;
            (*line) += 1;

            /* make longer line catch up until \n or EOF whichever comes first*/
            while (isEqual(*catchUpChar, *backN, 0) != 0)
            {
                *catchUpChar = fgetc(catchUpFile);
                if (*catchUpChar == EOF || *aheadChar == EOF)
                {
                    break;
                }
            }
}
