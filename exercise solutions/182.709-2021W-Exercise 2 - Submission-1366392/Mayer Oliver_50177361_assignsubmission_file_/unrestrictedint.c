/**
 * @file main.c
 * @author Mayer Oliver, 12023147
 * @brief implementation for unrestrictedint.h
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "unrestrictedint.h"
#include "revstr.h"

unsigned char *parse_Int_from_String(int *inp_size, int *signum)
{
    char *inp_number = NULL;
    unsigned char *dest;

    size_t size = 1;

    //--------------------- read line from stdin
    if ((*inp_size = getline(&inp_number, &size, stdin)) != -1)
    {
        if ((*inp_size == ENOMEM) || (*inp_size == EINVAL))
        {
            fprintf(stderr, "Failed reading line!");
            free(inp_number);
            exit(EXIT_FAILURE);
        }
    }

    //--------------------- check if input follows the size restrictions
    if (inp_number[0] == '\n')
        exit(EXIT_FAILURE);

    inp_number[*inp_size - 1] = '\0';
    if (*inp_size == 2)
    {
        if ((inp_number[0] == '-'))
        {
            fprintf(stderr, "Wrong input: \"%s\" is not a number!\n", inp_number);
            free(inp_number);
            exit(EXIT_FAILURE);
        }
    }
    if (inp_number[0] == '-')
    {
        fprintf(stderr, "Wrong input: \"%s\" is not a number!\n", inp_number);
        free(inp_number);
        exit(EXIT_FAILURE);
    }

    //--------------------- check if even or one digit
    if (((*inp_size - 1) % 2 != 0) && (*inp_size != 2))
    {
        fprintf(stderr, "Number of digits is not even!\n");
        exit(EXIT_FAILURE);
    }

    char help[*inp_size];
    int number_size1 = (*inp_size - 1);
    unsigned char number1[number_size1];

    //--------------------- reverse string so first digit is the first of the number
    revstr(help, inp_number);
    for (int i = 0; i < *inp_size - 1; i++)
    {
        //--------------------- check if only hex digits are in the number
        char c = tolower(help[i]);
        if ((c < 'a' || c > 'f') && (c < '0' || c > '9'))
        {
            fprintf(stderr, "Non hex digit (\" %c \") in %s!\n", c, inp_number);
            free(inp_number);
            exit(EXIT_FAILURE);
        }
        char test[] = {c, '\0'};
        number1[i] = (unsigned char)strtol(test, NULL, 16);
    }
    free(inp_number);

    //--------------------- return integer number
    dest = malloc(sizeof(unsigned char) * (*inp_size));
    if (dest == NULL)
    {
        return NULL;
    }
    for (int i = 0; i < number_size1; i++)
    {
        dest[i] = number1[i];
    }

    return dest;
}

unsigned char *addIntegers(unsigned char *num1, unsigned char *num2, int size_num1, int size_num2, int *ret_size)
{
    unsigned char overflow = 0;
    size_t size = 0;
    int cnt = 0;

    //--------------------- set the result size
    int resultSize = 0;
    if (size_num1 >= size_num2)
    {
        resultSize = size_num1;
    }
    else
    {
        resultSize = size_num2;
    }

    unsigned char *result = malloc(sizeof(unsigned char) * resultSize);
    if (result == NULL)
    {
        return NULL;
    }

    while (cnt < resultSize)
    {
        size = size + sizeof(unsigned char);

        //--------------------- if one number is smaller take 0-values
        unsigned char cur1 = 0;
        if (cnt < size_num1)
        {
            cur1 = num1[cnt];
        }

        unsigned char cur2 = 0;
        if (cnt < size_num2)
        {
            cur2 = num2[cnt];
        }

        //--------------------- add everything
        result[cnt] = overflow + cur1 + cur2;

        overflow = result[cnt] >> 4;
        if (overflow == 1)
        {
            result[cnt] = result[cnt] & 0x0F;
        }

        cnt++;
    }

    //--------------------- overflow after last addition
    if (overflow == 0x1)
    {
        resultSize++;
        result = realloc(result, resultSize * sizeof(unsigned char));
        if (result == NULL)
        {
            return NULL;
        }
        result[resultSize - 1] = 1;
    }

    //--------------------- if uneven number of digits in result
    if ((resultSize % 2 != 0) && resultSize != 1)
    {
        resultSize++;
        result = realloc(result, resultSize * sizeof(unsigned char));
        if (result == NULL)
        {
            return NULL;
        }
        result[resultSize - 1] = 0;
    }

    *ret_size = resultSize;
    return result;
}
