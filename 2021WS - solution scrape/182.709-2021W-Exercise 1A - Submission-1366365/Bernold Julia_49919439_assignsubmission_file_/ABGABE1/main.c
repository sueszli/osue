/**
 * @file main.c
 * @author Julia Bernold <12025986@student.tuwien.ac.at>
 * @date 31.10.2021
 * 
 * @brief main program module
 * 
 * This program checks whether or not an input is a palindrom
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

int s = 0;
int i = 0;
int c;
int pos_arg = 0;
char *o_arg = NULL;
int size = 10;
int overflowsize = 10;
char *buffer = NULL;
FILE *out_file = NULL;
char *name = NULL;
char *res = NULL;
char *overflow = NULL;

int palindrom(char line[]);
void read_from(FILE *file);
void options(char *line, char *res);
int main(int argc, char **argv)
{
    name = argv[0];
    buffer = malloc(size);
    overflow = malloc(size);
    if (overflow == NULL)
    {
        fprintf(stderr, "%s memory allocation failed: %s: \n", name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (buffer == NULL)
    {
        fprintf(stderr, "%s memory allocation failed: %s: \n", name, strerror(errno));
        free(overflow);
        exit(EXIT_FAILURE);
    }
    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        case 's':
            s = 1; //if s is passed, whitespaces are to be ignored
            break;
        case 'i':
            i = 1; //if i is passed, lower and uppercase letters shall be ignored
            break;
        case 'o':
            o_arg = optarg; //output file name
            break;
        case '?': //invalid option
            break;
        default:
            assert(0);
            free(buffer);
            free(overflow);
            return EXIT_FAILURE;
        }
    }

    if (o_arg == NULL) //output file
    {
        out_file = stdout;
    }
    else
    {
        out_file = fopen(o_arg, "w");
        if (out_file == NULL)
        {
            fprintf(stderr, "%s fopen failed: %s:\n", name, strerror(errno));
            free(overflow);
            free(buffer);
            exit(EXIT_FAILURE);
        }
    }

    pos_arg = argc - optind; //input file
    if (pos_arg == 0)
    {
        FILE *in_file = stdin;
        read_from(in_file);
    }
    else
    {
        for (int j = 0; j < pos_arg; j++)
        {
            FILE *in_file = fopen(argv[optind + j], "r");
            if (in_file == NULL)
            {
                fprintf(stderr, "%s fopen failed: %s:\n", name, strerror(errno));
                free(buffer);
                free(overflow);
                exit(EXIT_FAILURE);
            }
            read_from(in_file);
        }
    }
}

int palindrom(char line[])
{
    char line_rev[strlen(line) + 1];
    line_rev[strlen(line)] = '\0';
    for (int j = 0; j < strlen(line); j++)
    {
        line_rev[j] = line[strlen(line) - (j + 1)];
    }
    line_rev[strlen(line)] = '\0';
    if (strcmp(line_rev, line) == 0)
    {
        return 1;
    }
    return 0;
}

void read_from(FILE *file)
{

    while (fgets(buffer, size, file))
    {
        while (buffer[strlen(buffer) - 1] != '\n')
        {
            overflowsize = overflowsize + size;
            size = size * 2;

            buffer = realloc(buffer, size);
            if (buffer == NULL)
            {
                fprintf(stderr, "%s memory allocation failed: %s: \n", name, strerror(errno));
                free(overflow);
                exit(EXIT_FAILURE);
            }
            overflow = realloc(overflow, overflowsize);
            if (overflow == NULL)
            {
                fprintf(stderr, "%s memory allocation failed: %s: \n", name, strerror(errno));
                free(buffer);
                exit(EXIT_FAILURE);
            }
            strcat(overflow, buffer);
            buffer[strlen(buffer)] = '\0';
            overflow[strlen(overflow)] = '\0';
            fgets(buffer, size, file);
        }

        strcat(overflow, buffer);
        buffer[0] = '\0';
        strcat(buffer, overflow);
        char line[strlen(buffer) + 1];
        strncpy(line, buffer, strlen(buffer));
        line[strlen(buffer)] = '\0';
        char og_line[strlen(buffer) + 1];
        strncpy(og_line, buffer, strlen(buffer));
        og_line[strlen(buffer) - 1] = '\0';

        res = malloc(strlen(line) + 1);
        if (res == NULL)
        {
            fprintf(stderr, "%s memory allocation failed: %s: \n", name, strerror(errno));
            free(buffer);
            free(overflow);
            exit(EXIT_FAILURE);
        }

        options(line, res);

        int pal = palindrom(line);
        if (pal == 1)
        {
            fprintf(out_file, "%s is a palindrom\n", og_line);
        }
        else
        {
            fprintf(out_file, "%s is not a palindrom\n", og_line);
        }

        buffer[0] = '\0';
        overflow[0] = '\0';
        free(res);
    }
    free(overflow);
    free(buffer);
    fclose(file);
    exit(EXIT_SUCCESS);
}

void options(char *line, char *res)
{
    line[strlen(line) - 1] = '\0';
    if (i)
    { //all letters to lowercase
        for (int j = 0; j < strlen(line) - 1; j++)
        {
            line[j] = tolower(line[j]);
            // printf("LOWER Actual temp = <%s> length = <%ld>\n", line, strlen(line));
        }
    }
    if (s)
    { //get rid of spaces
        char temp[strlen(line)];
        // temp[strlen(temp)-1] = '\0';
        int k = 0;
        for (int j = 0; j < strlen(line); j++)
        {
            if (line[j] != ' ')
            {
                temp[k] = line[j];
                k++;
                temp[k] = '\0';
            }
        }
        line[strlen(line) - 1] = '\0';
        //printf("LINE %s\n", temp);
        strcpy(line, temp);
    }
}