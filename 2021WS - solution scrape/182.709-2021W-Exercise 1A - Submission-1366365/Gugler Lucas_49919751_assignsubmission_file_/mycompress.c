/**
 * @file mycompress.c
 * @author Lucas Gugler 12019849
 * @date 4.11.2021
 *
 * @brief Module implements functionality of mycompress
 **/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#define BUF_SIZE 1024

/**
 * Usage function.
 * @brief Prints an usage message to stderr and ends the program
 * @param prog_name Name of the program.
 **/
static void usage(const char prog_name[])
{
    fprintf(stderr, "[%s] Usage: %s [-o outfile] [file...]\n", prog_name, prog_name);
    exit(EXIT_FAILURE);
}

/**
 * Main function
 * @brief This function is the entrypoint of the mycompress program. It implements
 * the full functionality of the mycompress program. This includes argument handling, file handling,
 * compression of data and printing of statistics
 * @param argc
 * @param argv
 * @return Upon success EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 **/
int main(int argc, char *argv[])
{

    //read options
    const char *const progname = argv[0];
    char *o_arg = NULL;
    int b;

    while ((b = getopt(argc, argv, "o:")) != -1)
    {
        switch (b)
        {
        case 'o':
            o_arg = optarg;
            break;
        case '?': /* invalid option */
            usage(progname);
            break;
        default:
            // unreachable code
            assert(0);
        }
    }

    //open outputfile
    int c;
    FILE *output = stdout;
    if (o_arg != NULL)
    {
        output = fopen(o_arg, "w");
        if (output == NULL)
        {
            fprintf(stderr, "[%s]: fopen failed: %s\n", progname, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    int counter = 0;
    char curchar = ' ';
    int counterread = 0;
    int counterwrite = 0;

    int flag = 0;
    if (optind == argc)
    {
        flag = 1;
    }
    int pos = optind;

    //if stdin and stdout are used, characters would overlap
    if (flag == 1 && output == stdout)
    {
        //allocating memory for stdin input
        char buffer[BUF_SIZE];
        size_t contentSize = 1;
        char *content = malloc(sizeof(char) * BUF_SIZE);
        if (content == NULL)
        {
            fprintf(stderr, "[%s]: Failed to allocate content: %s\n", progname, strerror(errno));
            if (output != stdout)
            {
                fclose(output);
            }
            exit(EXIT_FAILURE);
        }
        content[0] = '\0';

        //saving input in content
        while (fgets(buffer, BUF_SIZE, stdin))
        {
            char *old = content;
            contentSize += strlen(buffer);
            content = realloc(content, contentSize);
            if (content == NULL)
            {
                fprintf(stderr, "[%s]: Failed to reallocate content: %s\n", progname, strerror(errno));
                free(old);
                if (output != stdout)
                {
                    fclose(output);
                }
                exit(EXIT_FAILURE);
            }
            strcat(content, buffer);
        }
        if (ferror(stdin))
        {
            free(content);
            fprintf(stderr, "[%s]: Error reading from stdin: %s\n", progname, strerror(errno));
            if (output != stdout)
            {
                fclose(output);
            }
            exit(EXIT_FAILURE);
        }

        fprintf(output, "\n");
        //compressing content
        for (int i = 0; i < strlen(content); i++)
        {
            char read = content[i];
            if (counter == 0)
            {
                curchar = read;
                counter = 1;
            }
            else if (curchar == read)
            {
                counter++;
            }
            else
            {
                fprintf(output, "%c%d", curchar, counter);
                curchar = read;
                counter = 1;
                counterwrite += 1 + floor(log10(abs(counter))) + 1;
            }
            counterread++;
        }
        free(content);
        //outputing last read element
        if (counter != 0)
        {
            fprintf(output, "%c%d\n", curchar, counter);
            counterwrite += 1 + floor(log10(abs(counter))) + 1;
        }
    }
    else
    {
        //reading each input file
        while (pos < argc || flag == 1)
        {
            FILE *input = stdin;
            if (flag == 0)
            {
                char *pathinput = argv[pos];
                input = fopen(pathinput, "r");
                if (input == NULL)
                {
                    fprintf(stderr, "[%s]: fopen failed: %s\n", progname, strerror(errno));
                    if (output != stdout)
                    {
                        fclose(output);
                    }
                    exit(EXIT_FAILURE);
                }
            }
            //parsing current input file
            while ((c = fgetc(input)) != EOF)
            {
                char read = (char)c;
                if (counter == 0)
                {
                    curchar = read;
                    counter = 1;
                }
                else if (curchar == read)
                {
                    counter++;
                }
                else
                {
                    //printing to output

                    fprintf(output, "%c%d", curchar, counter);

                    curchar = read;
                    counter = 1;
                    counterwrite += 1 + floor(log10(abs(counter))) + 1;
                }
                counterread++;
            }
            if (counter != 0)
            {
                fprintf(output, "%c%d", curchar, counter);
                counterwrite += 1 + floor(log10(abs(counter))) + 1;
                counter = 0;
            }
            pos++;
            fclose(input);
            counter = 0;
            flag = 0;
        }
    }

    //printing calculations
    float ratio = (float)counterwrite / (float)counterread * 100;

    fprintf(stderr, "\nRead: %d characters\nWritten: %d chararcets\nCompression ratio: %.1f%%\n", counterread, counterwrite, ratio);
    if (output != stdout)
    {
        fclose(output);
    }
    return EXIT_SUCCESS;
}
