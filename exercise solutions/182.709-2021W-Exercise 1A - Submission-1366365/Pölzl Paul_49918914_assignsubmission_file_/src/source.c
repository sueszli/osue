/**
 * @file source.c
 * @author Paul Pölzl (12022514)
 * @date 27.10.2021
 * 
 * @brief Implementation of the source module.
 */
#include "source.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/**
 * @brief Stores a lower case copy of the input string in output.
 * 
 * @param *src A pointer to the source string. (Input)
 * @param *dest A pointer to the destination string. (Output)
 */
static void str_tolower(char *src, char *dest)
{
    for (int i = 0; i < strlen(src); i++)
        dest[i] = tolower(src[i]);
}

/**
 * @brief Checks if the line contains the keyword.
 * 
 * @param *line The input string.
 * @param *keyword The keyword that will be searched in the line.
 * @param not_case_sensitive 0: Case sensitive; 1: Not case sensitve.
 * 
 * @return Returns 1 if the line contains the keyword and 0 otherwise.
 */
static int contains(char *line, char *keyword, int not_case_sensitive)
{
    char *line_cpy = strdup(line);          // Creates a pointer to a copy of string

    if (not_case_sensitive == 1)
    {
        str_tolower(keyword, keyword);      // Makes keyword lower case
        str_tolower(line, line_cpy);        // Stores a lower case version of line in line_cpy
    }

    if (strstr(line_cpy, keyword) != NULL)   // Returns 1 if line_cpy contains the keyword
        return 1;
        
    return 0;    
}

/**
 * @details global variables: myprog
 */
FILE *open(char *path, char *mode)
{
    FILE *fd;
    if ((fd = fopen(path, mode)) == NULL)
    {
        fprintf(stderr,"[%s] Error: fopen failed: %s.\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

/**
 * @details global variables: myprog
 */
void filter(FILE *in, FILE *out, char *keyword, int not_case_sensitive) 
{
    char *line = NULL;              // Pointer for the lines
    size_t size = sizeof(line);     // Size of the line array
    ssize_t len = 0;                // Length of the line array

    // Runs until all lines are read from in 
    while ((len = getline(&line, &size, in)) != EOF && quit == 0)
    {      
        // If a line contains the keyword it will be written to the output
        if (contains(line, keyword, not_case_sensitive) == 1)
        {
            if(fputs(line, out) == EOF)
            {
                fprintf(stderr, "[%s] Error: fputs failed: %s.\n", myprog, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

    // Check if an error occured in the getline function
    if (errno == EINVAL)
    {
        fprintf(stderr, "[%s] Error: getline failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    free(line);         // Free the memory allocated by getline
}

