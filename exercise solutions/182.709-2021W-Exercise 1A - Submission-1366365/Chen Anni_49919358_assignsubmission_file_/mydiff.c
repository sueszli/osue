/**
 * @name mydiff
 * @author Anni Chen (11902050)
 * @date   26.10.2021
 *
 * @brief A program that compares two files line by line and outputs how
 * many characters are different
 *
 * @details This program compares two files line by line and prints out the lines containing differences
 * as well as the number of differing characters either to stdout or a specified file.
 * Note that lines are only compared until one of them ends. Therefore the two lines 'abc\n' and 'abcdef\n'
 * are considered to be equal by this program.
 * This program also accepts lines of any length
 * The program receives two files to be compared as arguments as well as the following optional arguments:
 * [-i] to specify a case-insensitive comparison
 * [-o path] to specify that the output should be written to the specified file instead of stdout.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h> //including it because -std=c99 prevents <unistd.h> from being included
#include <strings.h>
#include <string.h>

//the name of the program
static char *name = "mydiff";

#define USAGE()                                                             \
    {                                                                       \
        fprintf(stdout, "[USAGE]: ./%s [-i] [-o outfile] file1 file2\n", name); \
        exit(EXIT_FAILURE);                                                 \
    }

#define ERROR_MSG(...)                           \
    {                                            \
        fprintf(stderr, "[%s] [ERROR]: ", name); \
        fprintf(stderr, __VA_ARGS__);            \
        fprintf(stderr, "\n");                   \
    }

#define ERROR_EXIT(...)         \
    {                           \
        ERROR_MSG(__VA_ARGS__); \
        exit(EXIT_FAILURE);     \
    }


/**
 * @brief min of two size values
 * @param a, first value
 * @param b, second value
 * @return returns the smaller value of both, if both are the same b is returned
 */
static size_t min(size_t a, size_t b)
{
    return a < b ? a : b;
}


/**
 * @brief number of characters that are different in two given lines
 * @details the function compares the two lines given as arguments character by charater
 * depending on whether the comparision is sensitive or insensitve, strcmp or strcasecmp is used
 * the comparision will stop if the newline character of one of the two lines is reached
 * @param insensitive, flag if set, the comparision is case insensitive meaning strcasecmp
 * is used, otherwise strcmp is used
 * @param l1, first line to be compared
 * @param l2, second line to be compared
 * @param charsToCompare, number of characters to be compared
 * @return the number of the charaters that are different in l1 and l2
 */
static int numOfDiffChars(int insensitive, char *l1, char *l2, size_t charsToCompare)
{

    int i;
    int count = 0;
    char char1[2] = "\0"; //convert char to string
    char char2[2] = "\0"; //convert char to string
    char c1;
    char c2;

    for (i = 0; i < charsToCompare; i++)
    {
        c1 = l1[i];
        c2 = l2[i];

        if (c1 == '\n' || c2 == '\n')
        {
            break;
        }

        char1[0] = c1;
        char2[0] = c2;

        if (insensitive)
        {
            if (strcasecmp(char1, char2) != 0)
            {
                count++;
            }
        }
        else
        {

            if (strcmp(char1, char2) != 0)
            {
                count++;
            }
        }
    }


    return count;
}

/**
 * @brief reads lines one by one of the two input files given and calls numOfDiffChars to compare the two lines
 * @details this function uses the getline function to read the lines, it further determines the minimum length
 * of the two lines to set a boundary for the comparision, it outputs the result in the output file specified
 * @param input1 the first input file
 * @param input2 the second input file
 * @param output the output file where the result will be showed
 * @param i flag that indicates whether it is a insensitive comparision or not
 *
 */
static void find_diff(FILE *input1, FILE *input2, FILE *output, int i)
{

    int line_count = 1;

    char *line_buf1 = NULL;
    size_t line_buf_size1 = 0;
    ssize_t line_size1;

    char *line_buf2 = NULL;
    size_t line_buf_size2 = 0;
    ssize_t line_size2;

    while (1)
    {

        //reads next line of inpu1
        //breaks if there is a failure or EOF is reached
        line_size1 = getline(&line_buf1, &line_buf_size1, input1);
        if (line_size1 == -1)
        {
            break;
        }

        //reads next line of inpu2
        //breaks if there is a failure or EOF is reached
        line_size2 = getline(&line_buf2, &line_buf_size2, input2);
        if (line_size2 == -1)
        {
            break;
        }

        //do the comparision here

        int diffChars;
        size_t line1Length = (size_t)line_size1;
        size_t line2Length = (size_t)line_size2;

        size_t charsToCompare = min(line1Length, line2Length);

        diffChars = numOfDiffChars(i, line_buf1, line_buf2, charsToCompare);

        if (diffChars != 0)
        {
            //write to stdout or outfile
            fprintf(output, "Line: %d, characters: %d\n", line_count, diffChars);
        }

        line_count++;
    }

    /* Free the allocated line buffer */
    free(line_buf1);
    line_buf1 = NULL;

    free(line_buf2);
    line_buf2 = NULL;


}

/**
 * @brief The entry point of the program.
 * @details This function executes the whole program.
 * It parses the arguments -i -o and the two positional arguments which are files,
 * then it calls find_diff to start the comparision
 * @return EXIT_SUCCESS if the comparision was successfull or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[])
{

    FILE *input1;
    FILE *input2;
    char *output = NULL;
    int c;
    int i = 0; //i initialized with 0 = false / 1 = true

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            i = 1;
            break;
        case 'o': output = optarg;
            break;
        case '?': /* invalid option */
            USAGE();
            break;
        default:
            USAGE();
        }
    }

    //if number of positional arguments is not 2 (file1 or file2 or both missing!)
    if ((argc - optind) != 2)
    {
        USAGE();
    }

    input1 = fopen(argv[optind], "r");
    if (input1 == NULL)
    {
        ERROR_EXIT("Unable to open file");
    }

    input2 = fopen(argv[optind + 1], "r");
    if (input2 == NULL)
    {
        ERROR_EXIT("Unable to open file");
    }

FILE *output_file;

    if(output == NULL){
        output_file = stdout;
    } else {
        output_file = fopen(output, "w");
        if (output_file == NULL)
    {
        ERROR_EXIT("Unable to open file");
    }
    }

    find_diff(input1, input2, output_file, i);

    fclose(input1);
    fclose(input2);
    fclose(output_file);

    return EXIT_SUCCESS;
}