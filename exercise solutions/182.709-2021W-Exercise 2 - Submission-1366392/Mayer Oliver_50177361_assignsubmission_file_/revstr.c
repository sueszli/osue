/**
 * @file main.c
 * @author Mayer Oliver, 12023147
 * @brief Implementation for revstr.h
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <string.h>
#include "revstr.h"

void revstr(char *dest, char *source)
{
    if (source == NULL)
    {
        exit(EXIT_FAILURE);
    }
    int size = strlen(source)+1;
    int i = 1;
    dest[size-1] = '\0';
    while (i < size)
    {
        dest[i-1] = source[size-(i+1)];
        i++;
    }
}