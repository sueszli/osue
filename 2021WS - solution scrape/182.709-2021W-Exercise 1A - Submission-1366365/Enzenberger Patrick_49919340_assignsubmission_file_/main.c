/**
 * @file main.c
 * @author Patrick Enzenberger <patrick.enzenberger@student.tuwien.ac.at>
 * @date 29.10.2021
 * 
 * @brief Main program module.
 * 
 * This program checks if the input from a file is a palindrom and writes the result to a file.
 * If there is no input file specified, input is taken from stdin. If there is no output file specified, the compressed data is written to stdout. 
 **/

#include "ispalindrom.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static char *myprog = "ispalindrom";

static void usage(void)
{
    fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]", myprog);
}

int main(int argc, char *argv[])
{
    FILE *in = stdin;
    FILE *out = stdout;
    int s = 0; //option ignore whitespaces
    int i = 0; // option ignore lower/upper case
    int c;

    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        case 'o':
            out = fopen(optarg, "w");
            if (out == NULL)
            {
                fprintf(stderr, "%s: fopen1 failed: %s\n", argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        case 'i':
            i = 1;
            break;
        case 's':
            s = 1;
            break;
        default:
            fprintf(stderr, "%s: invalid option: %s\n", argv[0], strerror(errno));
            usage();
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (optind == argc) //no input file given so stdin is used
    {
        ispalindrom(in, out, i, s);
    }

    while (optind < argc) //compress every input file
    {
        in = fopen(argv[optind], "r");
        if (in == NULL)
        {
            fprintf(stderr, "%s: fopen2 failed: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        ispalindrom(in, out, i, s);
        optind++;
    }

    fclose(in);
    fclose(out);

    return EXIT_SUCCESS;
}