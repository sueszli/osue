/**
 * @author 11908100 Elisabeth Losert
 * @date 28.10.2021
 * @brief TODO
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

int main(int argc, char *argv[])
{

    char opt;
    bool i_arg = false;
    char *o_arg = NULL;

    while ((opt = getopt(argc, argv, "io:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            i_arg = true;
            break;
        case 'o':
            o_arg = optarg;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-i case insensitive] [-o outfile] file1 file2\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if ((argc < 3) || (argc > 6))
    {
        fprintf(stderr, "Usage: %s [-i case insensitive] [-o outfile] file1 file2\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *outfile;

    if (o_arg != NULL)
    {
        outfile = fopen(o_arg, "w");
        if (outfile == NULL)
        {
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }

    FILE *file1 = fopen(argv[optind], "r");
    FILE *file2 = fopen(argv[optind + 1], "r");

    if ((file1 == NULL) || (file2 == NULL))
    {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    char *lineF1 = NULL;
    char *lineF2 = NULL;
    size_t len = 0;
    ssize_t readF1;
    ssize_t readF2;
    int lineNb = 1;
    int diff = 0;

    while (true)
    {
        readF1 = getline(&lineF1, &len, file1);
        readF2 = getline(&lineF2, &len, file2);
        if ((readF1 == -1) || (readF2 == -1))
        {
            break;
        }

        //compare lines and produce output
        for (size_t i = 0; i < MIN(strlen(lineF1), strlen(lineF2)) - 1; i++)
        {
            char s1[2] = {lineF1[i], '\0'};
            char s2[2] = {lineF2[i], '\0'};
            if (i_arg)
            {
                //case insensitive compare
                if (strcasecmp(s1, s2) != 0)
                {
                    diff++;
                }
            }
            else
            {
                //case sensitive compare
                if (strcmp(s1, s2) != 0)
                {
                    diff++;
                }
            }
        }

        if (diff != 0)
        {
            if (o_arg != NULL)
            {
                fprintf(outfile, "Line %i, charachters %i\n", lineNb, diff);
            }
            else
            {
                printf("Line %i, charachters %i\n", lineNb, diff);
            }
        }
        lineNb++;
        diff = 0;
    }

    fclose(file1);
    fclose(file2);
    if (o_arg != NULL)
    {
        fclose(outfile);
    }

    return EXIT_SUCCESS;
}