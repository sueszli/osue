/**
 * @file mygrepUtil.c
 * @author Laurenz Bilek e11904655@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Module with helper functions for Main
 **/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

//Usage message for getopt
void usage()
{
    fprintf(stdout, "Usage: mygrep [-i] [-o outfile] keyword [file...]");
}

//For caseinsensitive substring search
char *convertToLowerCase(char *str)
{
    size_t len = strlen(str);
    size_t i = 0;
    for (; i < len; ++i)
    {
        str[i] = tolower((unsigned char)str[i]);
    }
    return str;
}

//For writing a String into a file
void writeIntoFile(FILE *wf, char *line)
{
    if (wf == NULL)
    {
        fprintf(stderr, "[mygrep] Opening writing file failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(wf, "%s", line);
    //fwrite(line,1,sizeof(line),wf);
    //fputs(line, wf);
}

//To avoid duplicate code and to close open files
void closeFiles(FILE *rf, FILE *wf)
{
    if (rf)
    {
        fclose(rf);
    }
    if (wf)
    {
        fclose(wf);
    }
}

//1: opt_i   2: opt_o   3: o_arg   4: read File   5: keyword
void readFromFile(int opt_i, int opt_o, char *o_arg, char *file, char *keyword)
{
    FILE *rf;
    FILE *wf;
    char *line;
    size_t len;
    ssize_t read;

    rf = fopen(file, "r");
    if (rf == NULL)
    {
        fprintf(stderr, "[mygrep] Opening reading file failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    line = NULL;
    len = 0;
    read = getline(&line, &len, rf);
    //while there are still lines in the file
    while (read >= 0)
    {
        //if substring search is caseinsensitive
        if (opt_i)
        {
            line = convertToLowerCase(line);
            char *lower_keyword = convertToLowerCase(keyword);
            if (strstr(line, lower_keyword) != NULL)
            {
                //if a output file exists
                if (opt_o)
                {
                    wf = fopen(o_arg, "a");
                    writeIntoFile(wf, line);
                }
                else
                {
                    fprintf(stdout, "%s \n", line);
                }
            }
        }
        else if (strstr(line, keyword) != NULL)
        {

            if (opt_o)
            {
                wf = fopen(o_arg, "a");
                writeIntoFile(wf, line);
            }
            else
            {
                fprintf(stdout, "%s \n", line);
            }
        }
        read = getline(&line, &len, rf);
    }
    if (line)
    {
        free(line);
    }

    closeFiles(rf, wf);
}

//1: opt_i   2: opt_o   3: o_arg   4: keyword
void readFromStdin(int opt_i, int opt_o, char *o_arg, char *keyword)
{
    char *userInput;
    size_t len;
    ssize_t read;
    FILE *wf;

    userInput = NULL;
    len = 0;
    read = getline(&userInput, &len, stdin);

    //while there are still lines in stdin
    while (read >= 0)
    {

        //if substring search is caseinsensitive
        if (opt_i)
        {
            //copying Strings so the original userinput and keyword are preserved
            char userInputToCompare[sizeof(userInput)];
            strcpy(userInputToCompare, userInput);
            convertToLowerCase(userInputToCompare);
            char keywordToCompare[sizeof(keyword)];
            strcpy(keywordToCompare, keyword);
            convertToLowerCase(keywordToCompare);

            if (strstr(userInputToCompare, keywordToCompare) != NULL)
            {
                //if a output file exists
                if (opt_o)
                {
                    wf = fopen(o_arg, "a");
                    writeIntoFile(wf, userInput);
                }
                else
                {
                    fprintf(stdout, "%s \n", userInput);
                }
            }
        }
        else if (strstr(userInput, keyword) != NULL)
        {
            if (opt_o)
            {
                wf = fopen(o_arg, "a");
                writeIntoFile(wf, userInput);
            }
            else
            {
                fprintf(stdout, "%s \n", userInput);
            }
        }
        read = getline(&userInput, &len, stdin);
    }
    if (userInput)
    {
        free(userInput);
    }
    closeFiles(NULL, wf);
}