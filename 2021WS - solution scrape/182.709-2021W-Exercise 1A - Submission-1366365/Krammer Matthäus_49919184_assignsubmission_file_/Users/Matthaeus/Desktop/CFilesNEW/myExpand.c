#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>

static unsigned int tabstop = 8;
static char *outfile = NULL;
static unsigned int position = 0;

/**
 * @brief The program reads two flags, -t tabstop and -o outfile name, and an arbitrary amount of files in.
 * If any of those files contains tabs with the ASCII CODE "\t", the program will recognice it and substitute it with 
 * the correct amount of single spaces and output a file which contains all text of the previous read in files.
 * @author Matthaeus Krammer / 11904662
 * @date 08.11.2021
 * @param argc argument counter     
 * @param argv argument vector
 * @return int 
 * @details tabstop: the distance of one tab ("\t"), which can be changed with "-t [Value]", 
 *          *outfile: the variable to store the name of the output file, which can be set by "-o [Name]",
 *          position: the variable which marks the position in every row while traversing every file.         
 */

int main(int argc, char *argv[])
{

    FILE *filepointer1;
    FILE *filepointer2;

    int input = 0;
    input = getopt(argc, argv, "t:o");
    printf("%d\n", input);

    while (input != -1)
    {

        switch (input)
        {
        case 't':
            if (isdigit(*optarg))
            {
                printf("%s\n", optarg);
                tabstop = strtol(optarg, NULL, 0);
                if (tabstop != 0)
                {
                    printf("New tabstop: %i \n", tabstop);
                    input = getopt(argc, argv, "t:o");
                }
                else
                {
                    printf("Error: Tabstop cannot be 0");
                    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                printf("NaN\n");
                fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'o':
            outfile = argv[optind];
            optind++;
            input = getopt(argc, argv, "t:o");
            break;

        default:
            fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (outfile == NULL)
    {

        while ((argv[optind]) != NULL)
        {
            char *input_file = strdup(argv[optind]);
            filepointer1 = fopen(input_file, "r");
            if (filepointer1 != NULL)
            {

                char c;
                while ((c = fgetc(filepointer1)) != EOF)
                {

                    if (c == '\t')
                    {

                        int p = tabstop * ((position / tabstop) + 1);
                        while (position < p)
                        {

                            printf(" ");
                            position++;
                        }
                    }
                    else
                    {

                        (void)printf("%c", c);
                        if (c == '\n')
                        {
                            position = 0;
                        }
                        else
                        {
                            position++;
                        }
                    }
                }
            }
            optind++;
            fclose(filepointer1);
            printf("\n");
            free(input_file);
        }
    }
    else
    {
        while ((argv[optind]) != NULL)
        {
            char *input_file = strdup(argv[optind]);
            filepointer1 = fopen(input_file, "r");
            filepointer2 = fopen(outfile, "a");
            if (filepointer1 != NULL)
            {

                char c;
                while ((c = fgetc(filepointer1)) != EOF)
                {

                    if (c == '\t')
                    {

                        int p = tabstop * ((position / tabstop) + 1);
                        while (position < p)
                        {

                            c = ' ';
                            fputc(c, filepointer2);
                            position++;
                        }
                    }
                    else
                    {

                        fputc(c, filepointer2);
                        if (c == '\n')
                        {
                            position = 0;
                        }
                        else
                        {
                            position++;
                        }
                    }
                }
            }
            optind++;
            fputc('\n', filepointer2);
            fclose(filepointer1);
            fclose(filepointer2);
            free(input_file);
            printf("\n");
        }
    }
    exit(EXIT_SUCCESS);
}
