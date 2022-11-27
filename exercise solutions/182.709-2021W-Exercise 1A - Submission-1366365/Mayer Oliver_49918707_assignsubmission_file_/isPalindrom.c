/**
 * @file isPalindrom.c
 * @author Oliver Mayer, MatNr: 12023147
 * @brief Includes the implementation for isPalindrom.h
 * @version 0.1
 * @date 2021-11-09
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include "isPalindrom.h"

#define IS_PALINDROM "is a palindrom"
#define NOT_PALINDROM "is not a palindrom"

/**
 * @brief Changes the given string to only use lower case letters
 * 
 * @param dest place were the result is stored
 * @param source string which should be changed
 */
void toLowerCase(char *dest, char *source);

/**
 * @brief Removes whitespaces from the given string
 * 
 * @param dest place where the result is stored
 * @param source string which whitespaces should be removed
 * @return int 0 on success
 *             -1 on failure
 */
int removeWhitespaces(char *dest, char *source);

int removeWhitespaces(char *dest, char *source)
{
    char *help = malloc(sizeof(char) * strlen(source));
    char *head = help;
    if (help == NULL)
    {
        return -1;
    }
    while (*source != '\0')
    {
        if (*source != ' ')
        {
            *help = *source;
            help++;
        }
        source++;
    }
    *help = '\0';
    strcpy(dest, head);
    free(head);
    return 0;
}

void toLowerCase(char *dest, char *source)
{
    for (int i = 0; i < strlen(source); ++i)
    {
        dest[i] = (char)tolower(source[i]);
    }
}

char *isPalindrom(char *string, int ignoreWhitespaces, int caseSensitive)
{
    char *str = malloc(sizeof(char) * strlen(string)); //help string so the given string remains unchanged
    char *endOfString = NULL;
    char *startOfString = str;
    if (str == NULL)
    {
        return NULL;
    }

    strcpy(str, string);
    if (caseSensitive == 0)
    {
        toLowerCase(str, str);
    }
    if (ignoreWhitespaces == 1)
    {
        if (removeWhitespaces(str, str) == -1)
        {
            return NULL;
        }
    }

    //Run from start and end to the middle
    endOfString = str + strlen(str) - 1;
    while (endOfString > startOfString)
    {
        if (*endOfString != *startOfString)
        {
            free(str);
            return NOT_PALINDROM;
        }
        endOfString--;
        startOfString++;
    }
    free(str);
    return IS_PALINDROM;
}