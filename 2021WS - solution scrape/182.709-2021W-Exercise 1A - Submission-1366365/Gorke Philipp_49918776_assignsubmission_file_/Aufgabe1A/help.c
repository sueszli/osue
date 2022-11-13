/**
 * @file help.c
 * @author Philipp Gorke <e12022511@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief help functions for main.c
 * 
 * Includes random functions which are helpful for main
 *
 **/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "help.h"


/**
 * @brief compares two strings and returns the length of the smaller one
 * @details return value is the length of the smaller string
 */
int biggerString(char s1[], char s2[])
{
    int s1Length = strlen(s1);
    int s2Length = strlen(s2);

    if (s1Length > s2Length)
        return s2Length - 1;
    return s1Length - 1;
}


/**
 * @brief returns number of incorrect chars when two Strings are compared
 * @details Just counts number of mismatching characters in two Strings
 */
int incorrectChars(char s1[], char s2[], int max)
{
    int counter = 0;
    for (size_t i = 0; i < max; i++)
    {
        if (s1[i] != s2[i])
            counter++;
    }
    return counter;
}

/**
 * @brief formats String according to the problem
 * @details formats String for a single line like this
 * "Line: 3 characters: 3"
 */
void getString(int line, int characters, char *output)
{
    char s1[] = "Line: ", s2[] = " characters: ";
    char sLine[200], sChar[200];
    snprintf(sLine, 200, "%d", line);
    snprintf(sChar, 200, "%d", characters);
    strncat(output, s1, 7);
    strncat(output, sLine, 200);
    strncat(output, s2, 14);
    strncat(output, sChar, 200);
    strncat(output, "\n", 2);
  //  return output;
}