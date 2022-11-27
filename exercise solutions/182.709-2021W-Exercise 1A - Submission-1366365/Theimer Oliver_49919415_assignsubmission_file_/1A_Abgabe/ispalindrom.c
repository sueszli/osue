#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h> //strcspn
#include <ctype.h> //tolower
#include <stdbool.h> //boolean
#include <errno.h> //errno

/**
 * @brief checks if a string is a palindrom meaning that its the same word if you read it from the front or back
 * 
 * @param start pointer to the first character
 * @param end pointer to the last character
 * @param ignoreWhiteSpaces indicates if the check should ignore blank spaces
 * @param caseInsensitive indicated if the check should be case insensitive
 * @return int 
 */
int checkPalindrom(char* start, char* end, int ignoreWhiteSpaces, int caseInsensitive);

/**
 * @brief prints the input plus the answer of the program to an output
 * 
 * @param input pointer to the input file
 * @param output pointer to the output file
 * @param opt_s indicates if the -s flag is set leading to ignoreing white spaces
 * @param opt_i indicates if the -i flag is set leading to an case insensitive check
 */
void printAnswer(FILE *input, FILE *output, int opt_s, int opt_i);

int main(int argc, char* argv[]){
    
    char *o_arg = NULL;
    int opt_s = 0;
    int opt_i = 0;
    int c;

    while( (c = getopt(argc, argv, "sio:")) != -1){
        switch (c){
            case 'o': o_arg = optarg;
                break;
            case 's': opt_s++;
                break;
            case 'i': opt_i++;
                break;
            case '?':
                return EXIT_FAILURE;
                break;
            default: assert(0);
        }
    }

    if (opt_s > 1 || opt_i > 1){
        fprintf(stderr, "[%s] ERROR: invalid number of options\n", argv[0]);
        fprintf(stderr, "Usage: ispalindrom [-s] [-i] [-o outfile] [file...]\n");
        exit(EXIT_FAILURE);
    }

    FILE *output;
    if (o_arg != NULL){
        if((output = fopen(o_arg, "w")) == NULL){
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
        }
    } else{
        output = stdout;
    }

    char **input = argv + optind;
    FILE *inputFile;
    if (*input != NULL){
        while (*input != NULL){
            if((inputFile = fopen(*input, "r")) == NULL){
                fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                fclose(output);
                exit(EXIT_FAILURE);
            }
            printAnswer(inputFile, output, opt_s, opt_i);
            fclose(inputFile);
            input++;
        }
    } else {
        printAnswer(stdin, output, opt_s, opt_i);
    }
    fclose(output);
    return EXIT_SUCCESS;
}

void printAnswer(FILE *input, FILE *output, int opt_s, int opt_i){
    char *buffer;
    size_t len;
    int strLen;
    while ((strLen = getline(&buffer, &len, input)) > 1){
        //because in files the last line doesn't have a \n at the end
        strLen = strLen - (buffer[strLen - 1] == '\n'? 1 : 0);
        char *end = buffer + strLen - 1;
        if(buffer[strLen] == '\n'){
            buffer[strLen] = '\0';
        }
        //buffer[strcspn(buffer, "\n")] = 0;
        fprintf(output, 
                "%s is %s a palindrom\n",
                buffer,
                checkPalindrom(buffer, end, opt_s, opt_i) == 1? "": "not");
    }
    free(buffer);
}

int checkPalindrom(char* start, char* end, int ignoreWhiteSpaces, int caseInsensitive){
    if (ignoreWhiteSpaces){
        while(*start == ' '){
            start++;
        }
        while(*end == ' '){
            end--;
        }
    }
    
    while(!(start == end || (start - 1) == end)){
        if (*start != *end){
            if (caseInsensitive){
                if (tolower(*start) != tolower(*end)){
                    return 0;
                }
            } else{
                return 0;
            }
        }
        start++;
        end--;
        if (ignoreWhiteSpaces){
            while (*start == ' '){
                start++;
            }
            while (*end == ' '){
                end--;
            }
        }
    }
    return 1;
}