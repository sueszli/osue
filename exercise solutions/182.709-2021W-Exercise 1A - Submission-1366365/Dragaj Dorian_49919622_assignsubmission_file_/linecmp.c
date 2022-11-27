/**
 * @file linecmp.c
 * @author Dorian Dragaj <e11702371@student.tuwien.ac.at>
 * @date 03.11.2021
 * 
 * @brief Implementation of the linecmp module.
 *
 * It contains a function to compare the equality of two lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "linecmp.h"

/**
 * Compare two lines for equality.
 * @brief This function compares two lines for their equality and return the number of mismatching characters.
 * @details This function contains an option (opt_i) in the paramater to include case insensitivity for the comparison of the lines.
 * @param line1 The first line to be compared.
 * @param line2 The second line to be compared.
 * @param opt_i Option whether program should include case insensitivity. (if opt_i == 1, include otherwise not)
 * @return Returns number of different characters.
 */
int line_cmp(char *s1, char *s2, int opt_i)
{
    int char_count = 0, len1 = strlen(s1), len2 = strlen(s2), cap;

    if(s1[len1 - 1] == '\n')
    {
        len1--;
    }

    if(s2[len2 - 1] == '\n')
    {
        len2--;
    }

    cap = len1;

    if(len1 > len2)
    {
        cap = len2;
    }
    
    for(int i=0; i < cap; i++)  
    {
        if (opt_i == 1) {
            s1[i] = tolower(s1[i]);
            s2[i] = tolower(s2[i]);
        }
        if(s1[i] != s2[i])    
        {
            char_count++;
        }
 	}
    return char_count;
}

