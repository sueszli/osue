#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


/*
 *
 *  @file ispalindrom.c
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief The program reads line per line and checks if the line is a palindrom. Depending on the argument it either prints to stdout or into the specified file. As for input it takes the given files. If no file is given stdin is used.
 */


void checkPalindrom(char cpy[], FILE *output, char str[]);
void ignoreWhitespace(char str[]);
void toUpper(char str[]);
void reverse(char str1[], int index, int size);
void stringHandler(FILE *output, char str[], int i_flag, int s_flag);
void fileToLine(FILE *input, FILE *output, int i_flag, int s_flag);

int main(int argc, char ** argv){
    int o_flag=0;
    int s_flag=0;
    int i_flag=0;
    int opt;

    FILE * outFile = stdout;

    while((opt=getopt(argc,argv,"o:si"))!=-1){
        switch(opt){
            case 'o':
                o_flag++;
                outFile = fopen(optarg,"w");

                if (outFile == NULL) {
                    fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                s_flag++;
                break;
            case 'i':
                i_flag++;
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    int ind = optind;

    if (optind >= argc){ // no input file if true

        fileToLine(stdin, outFile, i_flag, s_flag);
    } else{
        FILE *inputFile;
        while (ind != argc){
            inputFile = fopen(argv[ind], "r");

            if (inputFile == NULL) {
                fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            fileToLine(inputFile, outFile, i_flag, s_flag);
            ++ind;
        }
        fclose(inputFile);
    }

    fclose(outFile);
    exit(EXIT_SUCCESS);
}



/*
 *
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief This method takes the input File (or stdin) and reads it one line at a time at calls the method stringHandler() with the line
 *  @param FILE *outputFile : where the results should be printed
 *  @param FILE *inputFile : input of the user
 *  @param int i_flag : flag if input should be case sensitive
 *  @param int s_flag : flag if whitespace should be ignored
 */
void fileToLine(FILE *input, FILE *output, int i_flag, int s_flag){
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    while ((nread = getline(&line, &len, input)) != -1) {
        if(line[strlen(line) - 1] == '\n'){
            line[strlen(line) - 1] = '\0';
        }


        stringHandler(output, line, i_flag, s_flag);
    }
    free(line);
}

/*
 *
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief This method takes a single line and calls toUpper or ignoreWhitespace depending on flags and calls checkPalindrom with the line
 *  @param FILE *outputFile : where the results should be printed
 *  @param char str[] : single line that should be checked if it is a palindrom
 *  @param int i_flag : flag if input should be case sensitive
 *  @param int s_flag : flag if whitespace should be ignored
 */
void stringHandler(FILE *output, char str[], int i_flag, int s_flag){
    char *cpy = strdup(str);

    if (cpy == NULL) {
        fprintf(stderr, "strdup failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (i_flag){
        toUpper(cpy);
    }

    if (s_flag){
        ignoreWhitespace(cpy);
    }

    checkPalindrom(cpy, output, str);
    free(cpy);
}

/*
 *
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief This method takes a single line and removes all whitespace characters
 *  @param char str[] : single line from which the whitespaces should be removed
 */
void ignoreWhitespace(char str[]){


    int i = 0, j = 0;
    while (str[i])
    {
        if (str[i] != ' ')
            str[j++] = str[i];
        i++;
    }
    str[j] = '\0';

}


/*
 *
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief This method takes a single line and turns all its characters into its capitalized version
 *  @param char str[] : single line which should be capitalized
 */
void toUpper(char str[]){


    int i = 0;
    while (str[i])
    {
        str[i] = toupper(str[i]);
        i++;
    }
    str[i] = '\0';

}


/*
 *
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief This method takes a single line and calls toUpper or ignoreWhitespace depending on flags and calls checkPalindrom with the line
 *  @param char cpy[] : copy of the line to be checked, might be capitalized
 *  @param FILE *outputFile : where the results should be printed
 *  @param char str[] : single line that gets used to print the result
 */
void checkPalindrom(char cpy[], FILE *output, char str[]){
    char *reverseStr = strdup(cpy);
    reverse(reverseStr, 0, strlen(cpy)-1);
    int x = 0;

    int i;
    int length = strlen(cpy);
    for(i = 0; i< length; i++){
        if(reverseStr[i] != cpy[i]){
            fprintf(output, "%s is not a palindrom\n", str);
            ++x;
            break;
        }
    }
    if (!x){
        fprintf(output, "%s is a palindrom\n", str);
    }
    free(reverseStr);
}


/*
 *
 *  @author Ensar Yabas (11809077)
 *  @date 09.11.2021
 *
 *  @brief Reverses the given string using recursion, each recursion swaps two characters
 *  @param char str[] : single line that should be reversed
 *  @param int index : index of which characters should be swapped
 *  @param int size : size of string without \n
 */
void reverse(char str1[], int index, int size) {
    char temp;

    temp = str1[index];
    str1[index] = str1[size - index];
    str1[size - index] = temp;

    if (index == size / 2)
    {
        return;
    }
    reverse(str1, index + 1, size);
}