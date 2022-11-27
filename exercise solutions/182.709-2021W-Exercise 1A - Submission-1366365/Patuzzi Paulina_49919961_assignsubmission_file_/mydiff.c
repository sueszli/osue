/**
 * @file mydiff.c
 * @author Paulina Patuzzi (01607360)
 * @date 2020-11-09
 *
 * @brief Implementation of mydiff module
 * */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>

char* program_name;


/**
 * @brief This function compares two given files for differences
 *
 * @param infile1 the file to compare with infile2
 * @param infile2 the file to compare with infile1
 * @param outfile The file to write the ressults to
 * @param ignoreCase A flag depending on which case will be ignored or not
 *
 * @return No return value
 * */
static void compare(FILE *infile1, FILE *infile2, FILE *outfile, int ignoreCase)
{
    char *line1 = NULL;
    char *line2 = NULL;
    char outline[100];
    size_t n = 0;
    int len1, len2;

    int linecnt = 1;
    int diffcnt;

    while (((len1 = getline(&line1, &n, infile1)) != -1) && ((len2 = getline(&line2, &n, infile2)) != -1)) {
         // get the length of the shorter line and subtract 1 because otherwise '\0' will be counted
        int minlen = ((len1 < len2) ? (len1) : (len2)) - 1;
        // reset difference counter and outline string:
        diffcnt = 0;
        outline[0] = '\0';
        for (int i = 0; i < minlen; i++) {
            // if case shall be ignored: use strncasecmp to check for differences, otherwise strncmp
            // and compare character by character
            if ((ignoreCase && (strncasecmp((line1 + i), (line2 + i), 1) != 0))
                    || (!ignoreCase && (strncmp((line1 + i), (line2 + i), 1) != 0))) {
                diffcnt++;
            }
        }

        // if different characters have been found write their line and number to the outfile
        if (diffcnt > 0) {
            sprintf(outline, "Line %d: %d different character(s)\n", linecnt, diffcnt);
            if (fputs(outline, outfile) == EOF) {
                fprintf(stderr, "%s: error on line %d: failed to write to output file: %s \n",
                            program_name, __LINE__, strerror(errno));
                exit(EXIT_FAILURE);
            };
            if (fflush(outfile) == EOF) {
                fprintf(stderr, "%s: error on line %d: failed to flush output file: %s \n",
                            program_name, __LINE__, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        linecnt++;
    }
    free(line1);
    free(line2);
}


int main(int argc, char *argv[])
{
    program_name = argv[0];
    char *outfile_name = NULL;
    FILE *outfile = stdout;
    FILE *infile1;
    FILE *infile2;
    int ignoreCase = 0;

    // check if options were given:
    int opt = -1;
    while ((opt = getopt(argc, argv, "io:")) != -1)
    {
        switch (opt)
        {
            case 'i':
                ignoreCase = 1;
                break;
            case 'o':
                outfile_name = optarg;
                break;
            default:
                // in case of invalid option print usage message
                fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2 \n", program_name);
                exit(EXIT_FAILURE);
        }
    }

    if (outfile_name != NULL) {
        if ((outfile = fopen(outfile_name, "w")) == NULL)
        {
            // error when opening the file
            fprintf(stderr, "%s: error on line %d: failed to open output file: %s \n",
                    program_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // check if files are given
    if ((optind + 2) == argc)
    {
        // open first infile
        if ((infile1 = fopen(argv[optind++], "r")) == NULL)
        {
            fprintf(stderr, "%s: error on line %d: failed to open input file: %s \n",
                    program_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // open second infile
        if ((infile2 = fopen(argv[optind], "r"))== NULL)
        {
            fprintf(stderr, "%s: error on line %d: failed to open input file: %s \n",
                    program_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // do actual comparison of the files
        compare(infile1, infile2, outfile, ignoreCase);

        // close first infile
        if (fclose(infile1) == EOF)
        {
            fprintf(stderr, "%s: error on line %d: failed to close input file: %s \n",
                    program_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

        // close second infile
        if (fclose(infile2) == EOF)
        {
            fprintf(stderr, "%s: error on line %d: failed to close input file: %s \n",
                    program_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }

    // in case of missing or too many arguments, print the usage message
    } else {
        fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2 \n", program_name);
        exit(EXIT_FAILURE);
    }

    // close outfile resp. stdout stream
    if (fclose(outfile) == EOF)
    {
        fprintf(stderr, "%s: error on line %d: failed to close output stream: %s \n",
                program_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

