/**
 * @file main.c
 * @author Dorian Dragaj <e11702371@student.tuwien.ac.at>
 * @date 03.11.2021
 *
 * @brief Main program module.
 * 
 * This program reads in two files and compares them.
 * If two lines differ, then the line number and the number of differing characters is printed.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "linecmp.h"

char *myprog; /** The program name.*/
int prog_status = 0; /** The program status, if below 0 we exit with a failed status.*/

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 */
void usage(void) 
{
    fprintf(stderr,"Usage: %s [-i] [-o outfile] file1 file2\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Function for closing and freeing resources.
 * @brief This function closes and frees all possible resources which could have been open, if opened.
 * @param file1 The first file to be compared.
 * @param file2 The second file to be compared.
 * @param output The file which contains the result of the compared two files.
 * @param line1 An allocated space for the lines of file1.
 * @param line2 An allocated space for the lines of file2.
 * @param o_arg The filename where the results of the comparison are saved.
 */
void close_resources(FILE *file1, FILE *file2, FILE *output, char *line1, char *line2, char *o_arg)
{
    if (file1 != NULL)
    {
        fclose(file1);
    }

    if (file2 != NULL)
    {
        fclose(file2);
    }

    if ((o_arg != NULL) && (output != NULL))
    {
        fclose(output);
    }

    if (line1 != NULL)
    {
        free(line1);
    }

    if (line2 != NULL)
    {
        free(line2);
    }
}

/**
 * Stream error handler.
 * @brief This function - if existing - handles stream errors, also closes all opened resources
 * @details global variables: myprog
 * @param file1 The first file to be compared.
 * @param file2 The second file to be compared.
 * @param output The file which contains the result of the compared two files.
 * @param line1 An allocated space for the lines of file1.
 * @param line2 An allocated space for the lines of file2.
 * @param o_arg The filename where the results of the comparison are saved.
 */
void handle_stream_err(FILE *file1, FILE *file2, FILE *output, char *line1, char *line2, char *o_arg) 
{   
    if ((file1 == NULL) || (file2 == NULL) || ((o_arg != NULL) && (output == NULL)))
    {
        close_resources(file1, file2, output, line1, line2, o_arg);
        fprintf(stderr, "%s: fopen failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters.
 * This function reads in two files and compares them.
 * If two lines differ, then the line number and the number of differing characters is printed.
 * Depending on the option arguments -i and -o, 
 * the lines are compared as case insensitive and the result is printed in a file instead to stdout.
 * @details global variables: myprog, prog_status
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *argv[]) 
{
    myprog = argv[0];
    char *o_arg = NULL;
    int opt_i = 0;
    int c;
    while ((c = getopt(argc, argv, "o:i")) != -1) {
        switch (c) {
            case 'o': o_arg = optarg;
            break;
            case 'i': opt_i++;
            break;
            default: usage();
            break;
        }
    }

    if (opt_i > 1) 
    {
        fprintf(stderr,"%s: option ’i’ occurs more than once\n", myprog);
        usage();
    }

    if ((argc - optind) != 2) 
    {
        fprintf(stderr,"%s: number of positional arguments is not 2\n", myprog);
        usage();
    }

    const char *filePath1 = argv[optind], *filePath2 = argv[optind+1];
    FILE *file1, *file2, *output;

    file1 = fopen(filePath1, "r");
    file2 = fopen(filePath2, "r");
    if(o_arg != NULL)
    {
        output = fopen(o_arg, "w");
    }
    handle_stream_err(file1, file2, output, NULL, NULL, o_arg);

    char *line1 = NULL, *line2 = NULL;
    size_t len1 = 0, len2 = 0;
    ssize_t read1, read2;
    int line_count = 0;
    
    while (((read1 = getline(&line1, &len1, file1)) != -1) && ((read2 = getline(&line2, &len2, file2)) != -1)) 
    {
        int char_count = line_cmp(line1, line2, opt_i);   
        line_count++;

        if (char_count > 0)
        {
            if (o_arg == NULL) {
                printf("Line: %d, characters: %d\n", line_count, char_count);
            } else {
                fprintf(output, "Line: %d, characters: %d\n", line_count, char_count);
            }
        }
    }

    if ((feof(file1) == 0) && (read1 <= 0)) 
    {
        fprintf(stderr, "%s: reading line of first input file failed: %s\n", myprog, strerror(errno));
        prog_status = -1;
    }
    
    if ((feof(file2) == 0) && (read2 <= 0)) 
    {
        fprintf(stderr, "%s: reading line of second input file failed: %s\n", myprog, strerror(errno));
        prog_status = -1;
    }

    close_resources(file1, file2, output, line1, line2, o_arg);

    if (prog_status < 0) {
        exit(EXIT_FAILURE);    
    } else {
        exit(EXIT_SUCCESS);
    }
}