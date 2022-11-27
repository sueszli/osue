/**
*@file ispalindrom.c
*@author daniel brauneis 12021357
*@details checks whether strings are palindroms
*@date 2021-11-06
*Main Program
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

static int checkifpalindrom(char *line);
static void remove_spaces(char *str);
static void lowerCase( char *str);
static void usage(char *myprog);
static void writeOutput(char *ovalue, char *line, char *originalLine, FILE *f);
static void manipulateString(int opt_s, int opt_i, char *line);


/**
* usage Function
* @brief displays the right structur of programcall
* @param myprog contains name of program
*/
static void usage(char *myprog){
    fprintf(stderr, "Usage: %s [-s] [-i] [-o file] [file]", myprog);
    exit(EXIT_FAILURE);
}

/**
* Main Function
* @brief This is the main function of the program
* @param arc: number of command-line arguments
* @param argv: list of arguments
*/
int main(int argc, char **argv){
    char *myprog = argv[0];    
    //line to be checked
    char *line = NULL;
    //value of o-parameter
    char *ovalue = NULL;
    //outputfile
    FILE *f = NULL;
    //counter for tracking the iteration of argv (used for getting the inputfiles)
    int counter = 0;

    //bools for arguments
    int s_opt = 0;
    int i_opt = 0;
    int c = 0;
    while( (c = getopt(argc, argv, "sio:")) != -1){
        counter++;
        switch (c){
            case 's': s_opt = 1;
            break;
            case 'i': i_opt = 1;
            break;
            case 'o':
            ovalue = optarg;
            //create new file or overwrite given file
            f = fopen(ovalue, "w");
            fclose(f);
            break;
            case '?': usage(myprog);
            break;
            default:
            break;
        }
    }
    //to get counter to index of input files
    counter ++;
    //if an outputfile is given, counter has to be incremented once more
    if(ovalue != NULL) counter++;
    if(counter >= argc){// no input files
        //used to represent size of an object
        size_t n = 0;
        getline(&line, &n, stdin);    
        line[strlen(line)-1] = '\0';
        char originalLine[strlen(line)];
        strcpy(originalLine,line);
        manipulateString(s_opt, i_opt, line);
        writeOutput(ovalue, line, originalLine, f);
        exit(EXIT_SUCCESS);
    }
    while (counter < argc)
    {
        FILE *input = fopen(argv[counter], "r");
        if(input == NULL){//incase user provides invalid input filename
            fprintf(stderr, "The given input file: %s doesn't exists.\n", argv[counter]);
            exit(EXIT_FAILURE);
        } 
        //used to represent size of an object
        size_t len = 0;
        //same as size_t but able to represent the number -1 (error)
        ssize_t read;
        while ((read = getline(&line, &len, input)) != -1) {
            line[strlen(line)-1] = '\0';
            char originalLine[strlen(line)];
            strcpy(originalLine,line);
            manipulateString(s_opt, i_opt, line);
            writeOutput(ovalue, line, originalLine, f);
        }
        counter++;
    }
    fclose(f);
    exit(EXIT_SUCCESS);
}

/**
* manipulateString Function
* @brief removes spaces and transforms string line to lowercase, if needed
*/
static void manipulateString(int s_opt, int i_opt, char *line){
    if(s_opt) remove_spaces(line);
    if(i_opt) lowerCase(line);
}

/**
* writeOutput Function
* @brief writes output either to stdout or to given outputfile
* @param ovalue: outputfilename
* @param line: modified line (-i -s)
* @param originalLine: unmodified line
* @param f: outputfile
*/
static void writeOutput(char *ovalue, char *line, char *originalLine, FILE *f){
    if(ovalue != NULL){
            f = fopen(ovalue, "a");
            char *strToPrint = ( char * ) malloc( 80 * sizeof( char ) );
            checkifpalindrom(line) ? snprintf(strToPrint, strlen(originalLine)+17, "%s is a palindrom\n", originalLine) : snprintf(strToPrint, strlen(originalLine)+21, "%s is not a palindrom\n", originalLine);
            fputs(strToPrint, f);
        }else{
            checkifpalindrom(line) ? printf("%s is a palindrom\n", originalLine) : printf("%s is not a palindrom\n", originalLine);
        }
}

/**
* checkifpalindrom Function
* @brief checks if given string is a palindrom
* @param line str to check
* @return 1 if the given string is a palindrom, 0 if otherwise
*/
static int checkifpalindrom(char *line){
    for(int i = 0; line[i]; i++){
        int index = strlen(line)-i-1;
        char a = line[i];
        char b = line[index];
        if(a != b){
            return 0;
        } 
    }
    return 1;
}

/**
* remove_spaces Function
* @brief Function to remove all spaces from a given string
* @param str to remove spaces from
*/
static void remove_spaces(char *str)
{
    int counter = 0;
    for (int i = 0; str[i]; i++){
        if (str[i] != ' '){
            str[counter++] = str[i];
        }
    }
    str[counter] = '\0';
}

/**
* lowerCase Function
* @brief replaces all  Uppercase characters of given str with corresponding lowercase characters
* @param str to replace
*/
static void lowerCase(char *str){
    for(int i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
}
