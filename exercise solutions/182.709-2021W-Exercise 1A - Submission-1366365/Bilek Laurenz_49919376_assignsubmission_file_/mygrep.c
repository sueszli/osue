/**
 * @file mygrep.c
 * @author Laurenz Bilek e11904655@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Main Module with argument parsing
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "mygrepUtil.h"

int main(int argc, char *argv[])
{

    // mygrep [-i] [-o outfile] keyword [file...]

    char *o_arg = NULL;
    char *keyword = NULL;
    int opt_i = 0;
    int opt_o = 0;
    int c = 0;

    //put ':' in the starting of the
    //string so that program can
    //distinguish between '?' and ':'

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            if (opt_i)
            {
                usage();
            }
            else
            {
                opt_i++;
            }
            break;
        case 'o':
            if (opt_o)
            {
                usage();
            }
            else
            {
                o_arg = optarg;
                opt_o++;
            }
            break;
        case '?':
            printf("unknown option: %c \n", optopt);
            break;
        default:
            usage();
        }
    }

    if (opt_i == 1 && opt_o == 1)
    {
        keyword = argv[3];
    }
    else if (opt_i == 1 && opt_o == 0 || opt_i == 0 && opt_o == 1)
    {
        keyword = argv[2];
    }
    else
    {
        keyword = argv[1];
    }

    if ((optind+1) < argc) // If files have been handed over
    {
        while ((optind+1) < argc)
        {
            readFromFile(opt_i, opt_o, o_arg, argv[(optind+1)], keyword);
            optind++;
        }
    }
    else // If no files have been handed over
    {
       readFromStdin(opt_i, opt_o, o_arg, keyword);
    }

    return 0;
}