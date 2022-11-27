/**
* @author Adela-Iulia Georgescu, 11810894
* @date 29.10.2021
* @brief reads files line by line and for each line check whether it contains the search
        term keyword. 
* @details The line is printed if it contains keyword, otherwise it is not printed 
**/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

//@brief shows the user how to use the myProgram
static void usage(void);

//@brief process the option given, if there is an outputFile file or if it is case insensitive
static void processOptions(char opt);

//@brief works with the lines given to find the keyword
static void processLines(FILE *inputFile, FILE *outputFile, char *keyword);

//@brief returns 1 if the keyword is found, otherwise 0
static int found(char *text, char *keyword);

//@brief saves in newtext the initial text, but with all letters as lower case
static void caseinsensitive(char *text, char *newtext);

FILE *outputFile; //will hold the outputFile-file as a FILE-stream
char *myProgram; //the name of the program is written here
int ioption; //option case insesitive 
char *keyword; //the keyword to be searched in text

int main(int argc, char *argv[]){

    outputFile = stdout;
    myProgram = argv[0];
    ioption = 0;

    int opt; //for the current option character
    while ((opt = getopt(argc, argv, "io:")) != -1){
        processOptions(opt);
    }

    keyword = argv[optind++];

    FILE *inputFile = NULL;

    //some files might not exist, so we check them
    for(int i=optind; i<argc; i++){
        inputFile = fopen(argv[i],"r");
        if (inputFile == NULL){
            usage();
        }
        fclose(inputFile);
        inputFile = NULL;
    }

    //read from stdin if there is no file given
    if(optind == argc){
        processLines(stdin, outputFile, keyword);
    }
    else{
        for(int i=optind; i<argc; i++){
            
            //open file to read it 
            inputFile = fopen(argv[i],"r");
                    
            //check if the file exists
            if (inputFile == NULL){
                usage();
            }

            processLines(inputFile, outputFile, keyword);

            //close the file
            fclose(inputFile);
        }
    }

    //if there was given an output file, close it
    if(outputFile != stdout){
        fclose(outputFile);
    }
    exit(EXIT_SUCCESS);
}

void usage(void){
    fprintf(stderr, "%s: Usage: mygrep [-i] [-o outfile] keyword [file...]", myProgram);
    exit(EXIT_FAILURE);
}

void processOptions(char opt){
    switch(opt)
    {
        case 'i':  
                ioption = 1;
                break;
            case 'o': 
                //instead of stdout, we are writing the solution to the output file
                outputFile = fopen(optarg, "w");
                if ( outputFile == NULL ){
                    usage();
                }
                break;  
            case '?': 
                usage();
            default:
                assert(0);
    }
}

void processLines(FILE *inputFile, FILE *outputFile, char *keyword){
    char *currLine = NULL;
    size_t currLineSize = 0;
    ssize_t remaining;
    char newLine[100000];
    char newKeyword[100000];

    //get the first line
    remaining = getline(&currLine, &currLineSize, inputFile);
    int keyFound = 0;

    while (remaining >= 0){

        if (ioption == 1){
            caseinsensitive(currLine, newLine); //we transform the line to lower case
            caseinsensitive(keyword, newKeyword); // we transform the keyword to lowercase
            if(found(newLine, newKeyword) == 1){
                keyFound = 1;
            }
        }
        else{

            if(found(currLine, keyword) == 1){
                keyFound = 1;
            }

        }

        if(keyFound == 1){
            fputs(currLine, outputFile);
        }

        remaining = getline(&currLine, &currLineSize, inputFile);
        keyFound = 0;
    }

}

int found(char *text, char *keyword){
    char *ptr = strstr(text, keyword);
    if(ptr!=NULL){
        return 1;
    }
    return 0;
}
void caseinsensitive(char *text, char *newtext){
    int i = 0;
    strcpy(newtext, text);
    int len = strlen(text);
    while(i < len){
        newtext[i] = tolower(text[i]);
        i++;
    }
}
