/**
 * @file ispalindrom.c
 * @author Alyssa Scheer 11909736
 * @date 12.11.2021
 * @brief Checks if given strings are palindroms
 * @details Different options (-s, -i) cause the program to ignore whitespaces
 * or be case insensitive
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int inputFile(char* path);
int checkpalindrom(char* line);

typedef char bool;
#define true 1
#define false 0

/*
 * \var To pass on option to function, which checks for palindroms
 */
bool ignore_case = false;
bool ignore_whitespaces = false;

FILE* out = NULL;

int main (int argc, char* argv[])
{
    /*
     * \var set out to standard out
     */
    out = stdout;

    if(argc == 1)
    {
        printf("Not enough args");
        return -1;
    }

    /*
     * Check which option is given
     */
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-s") == 0)
            ignore_whitespaces = true;
        else if(strcmp(argv[i], "-i") == 0)
            ignore_case = true;
        else if(strcmp(argv[i], "-o") == 0)
        {
            /*
             * get index of path/ after -o txt file is given (also in argv)
             */
            i++;
            out = fopen(argv[i], "w+");
            if(out == NULL)
                exit(-1); //terminate programm
        }
        else
        {
            /*
             * if there are no more options (-s, -i, -o) next is the file to read
             */
            inputFile(argv[i]);
        }
    }

    fclose(out);
    return 0;
}

int inputFile(char* path)
{
    FILE* fd = fopen(path, "r");

    /*
     * terminates if
     */
    if(fd == NULL)
        exit(-1);

    char* line = NULL;
    size_t len = 0;
    ssize_t n;

    while((n = getline(&line, &len, fd)) != -1)
    {
        printf("%s", line);
        checkpalindrom(line);
        line = NULL;
        len = 0;
    }

    fclose(fd);
    return 0;
}

/*
 * Checks if line is a palindrom
 */
int checkpalindrom(char* line) {
    char* delete = line;
    int front = 0;
    int back = strlen(line) -1;

    /*
    * if -i, put all chars to lower case so it doesn't differentiate 
    */
    int i;
    int i2;
    if (ignore_case) {
        for (i = 0; i<=back; i++) {
            line[i]=tolower(line[i]);
        }
    }

    /*
    * if -i, delete whitespaces from string
    */
    if (ignore_whitespaces) {
        for (i2 = 0; i2<=back; i2++) {
            if (*delete == ' ') {
                ++delete;
            }
        }
    }

    /*
    * checks for palindrom
    */
    while (front > back) {
        if (line[front++] != line[back--]) {
            printf("%s is not a palindrome\n", line);
            return false;
        }
    }
    printf("%s is a palindrome\n", line);
    return true;
}