/**
 * @file mygrep.c
 * @author Josef Taha <e11920555@student.tuwien.ac.at>
 * @date 03.11.2021
 *
 * @brief module for reading lines from files and writing them in an output if it contains a given keyword
 *
 **/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include "header.h"



/**
 * A variant of fgets() with no fixed size length.
 *
 * @brief This functions gets the whole next line of a file without having to know the size of the line.
 * @param input the file from which lines are read.
 * @return a pointer to the read line and NULL if the end of the file is reached
 */
static char *adapGet(FILE *input) {
    
    #define BUFFERLENGTH 5

    char *line = NULL;
    int lineleng = 0;
    int bufleng = 0;
    char *tempbuf = malloc(BUFFERLENGTH);

    do {
        if (fgets(tempbuf, BUFFERLENGTH, input) == NULL) return NULL;
        bufleng = strlen(tempbuf);
        line = realloc(line, lineleng + bufleng + 1);
        strcpy(line + lineleng, tempbuf);
        lineleng += bufleng;
    } while (tempbuf[BUFFERLENGTH - 2] != '\n' && bufleng == BUFFERLENGTH - 1);

    free(tempbuf);
    return line;

}


/**
 * Decides whether a string contains a keyword.
 *
 * @brief This function returns 1 if a string (*line parameter) contains a keyword (*keyword parameter), 
 * else 0. By defining the i-flag with 0 or 1 the function does (not) ignore lower/uppercase.
 * @details The function does not check if the flag parameters values are 0 or 1.
 * @param line the string to check if it contains a keyword.
 * @param keyword the string to check if it is contained in a line.
 * @param iflag ignores lower/uppercase of characters in line if its value is 1
 * @return 1 if line contains keyword else 0
 */
static int containsKeyword(char *line, char *keyword, int iflag) {

    char *edit = malloc(strlen(line));
    strcpy(edit, line);

    if (iflag == 1) {
        for (int i = 0; edit[i]; i++) {
            edit[i] = tolower(edit[i]);
        }
    }

    if(strstr(edit, keyword) == NULL) return 0;
    
    return 1;
}



int io(FILE **input, FILE *output, char *keyword, int iflag, char *prog) {

    for (int i = 0; input[i]; i++) {

        char *line;

        while ((line = adapGet(input[i]))){

            if(containsKeyword(line,keyword,iflag)){

                fwrite(line, sizeof(char), strlen(line), output);
                if (output == NULL) {
                    fprintf(stderr, "[%s] fwrite failed: %s\n", prog, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                fflush(output);

            }

            free(line);
        }

    }
    return 0;
}
