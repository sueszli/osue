/**
 * @file ispalindrom.c
 * @author Patrick Enzenberger <patrick.enzenberger@student.tuwien.ac.at>
 * @date 9.11.2021
 * 
 * @brief prints if lines are palindrom.
 * 
 * This program checks if the input from a file is a palindrom and writes the result to a file.
 * If there is no input file specified, input is taken from stdin. If there is no output file specified, the compressed data is written to stdout. 
 **/

#include "ispalindrom.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *ispalindrom(FILE *in, FILE *out, char ignoreCaps, char ignoreWhitespace)
{
    char *line = NULL; //for getline
    size_t len = 0;    //for getline
    int read = 0;      //for getline
    int palindrom = 1; //1 if palindrom, 0 if not

    //check line by line
    while ((read = getline(&line, &len, in)) != -1)
    {
        //cut '\n'
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }

        //save original line to print it later
        char lineToPrint[strlen(line) + 1];
        strcpy(lineToPrint, line);
        
        //ignore whitespaces, option -s
        if ((ignoreWhitespace == 1))
        {
            int k = 0;

            for (int i = 0; i < strlen(line) - 1; i++)
            {
                if (line[i] != ' ')
                {
                    line[k] = line[i];
                    k++;
                }
            }
        }

        //ignore Caps, option -i
        if ((ignoreCaps == 1))
        {
            for (int i = 0; i < strlen(line); i++)
            {
                line[i] = tolower(line[i]);
            }
        }

        //check if palindrom
        for (int i = 0; strlen(line) / 2 >= i; i++)
        {
            //always palindrom if just one char
            if (strlen(line) <= 1)
            {
                break;
            }

            //palindrom = 0 if line is not a palindrom
            if (line[i] != line[strlen(line) - i - 1])
            {
                palindrom = 0;
            }
        }

        //print result of current line
        if (palindrom == 1)
        {
            fprintf(out, "%s is palindrom\n", lineToPrint);
        }
        else
        {
            fprintf(out, "%s is not palindrom\n", lineToPrint);
        }

        //set palindrom variable for next line
        palindrom = 1;
    }

    return out;
}