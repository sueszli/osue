/**
 *@name: mydiff.c
 *@author: Kevin Wielander, 11908531
 *@brief: This module compares each line between two files
 *@date: 25.10.2021
**/
#define _GNU_SOURCE // required by getline

#include <stdio.h>   // used for I/O
#include <stdlib.h>  // Standard library
#include <getopt.h>  //used for getopt
#include <assert.h>  // used for defaultcase (getopt)
#include <errno.h>   // used for errno
#include <string.h>  // used for strerror
#include <stdbool.h> //used for boolean
#include <ctype.h>   //used for toLower function (caseSensitivity)

// declared globally for usage function
char *myprog;

/**
 *@brief: prints the synopsis of the mydiff command
 *@details: terminates with exit_code 0, global variable myprog is used     
**/
static void usage(void){
    fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2 \n",myprog);
    exit(EXIT_FAILURE);
}

/**
 *@brief: compares two strings up to n chars and returns difference
 *@details: if there is no difference between those strings in the first n chars and returns the number of different characters
 *@param: str0 - first input string 
 *@param: str0 - second input string
 *@param: n - number of chars to be compared
 *@param: casesensitive - whether to differentiate between lower and upper case letters
 *@return: number of differnt characters between the input files      
**/
static int compareStrings(char *str0, char *str1, int n, bool casesensitive){
    int ctr = 0; // counts number of differences between two chars
    for(int i = 0; i < n; i++){
        int s0 = str0[i];
        int s1 = str1[i];
        if(casesensitive == false){
            s0 = tolower(s0);
            s1 = tolower(s1);
        }
        if(s0 != s1){
            ctr++;
        }
    }
    return ctr;
}
/**
 *@brief prints difference between each line
 *@details if there is at least one difference the formatted output is printed either to stdout or define out file
 *@param line the current line number
 *@param diffs number of differences occured between two lines
 *@param output if value is NULL printed to stdout othewrwise printed to specified path 
**/
static void formatOutput(int line, int diffs, FILE* output){
    if(diffs > 0){
        if(output == NULL){
            printf("Line: %i, characters: %i\n",line,diffs);
        }
        else{
            fprintf(output,"Line: %i, characters: %i\n",line,diffs);
        }
    }
}

/**
 *@brief: compares two files
 *@details: compares two files line by line and writes output either to stdout or defined output file
 *@param: path0 - input path of the first file
 *@param: path1 - input path of the second file
 *@param: casesensitive - differentiates between lower and upper case 
 *@param: outpath - path to file where output should be written if not null
**/
static void compare(char *path0, char *path1, bool casesensitive, char* outpath){
    FILE *input0 = fopen(path0, "r"); // tries to opens inpult file 1 in read Mode
    FILE *input1 = fopen(path1, "r"); // tries to opens inpult file 2 in Read Mode
    FILE* output = fopen(outpath, "a"); // tries to opens outputfile append Mode
 
    char *line0 = NULL; // defines buffer for a line from input0
    char *line1 = NULL; // defines buffer for a line from input1
    size_t size = 0;  // size for getline
    size_t lineCtr = 0; // keeps track of line number during each iteration used for output
    int diffChars = 0; // gets the different characters from compareStrings function

    if(input0 == NULL || input1 == NULL){
        fprintf(stderr, "%s - fopen failed: %s\n", myprog, strerror(errno));
        exit(EXIT_FAILURE);
    }


    while(
        ((getline(&line0, &size, input0)) != -1) 
        && 
        ((getline(&line1, &size, input1)) !=1)
        ){
        int lineLength0 = strlen(line0);
        int lineLength1 = strlen(line1);
        lineLength0 = (lineLength1 < lineLength0) ? lineLength1 : lineLength0;
        diffChars = compareStrings(line0, line1, lineLength0-1, casesensitive);
        formatOutput(++lineCtr,diffChars,output);

    }
    
    //close resources 
    fclose(input0); 
    fclose(input1);
    if(outpath != NULL){
        fclose(output);
    }

    free(line0);
    free(line1);

    exit(EXIT_SUCCESS);  
}


int main(int argc, char *argv[])
{

myprog = argv[0]; //program name
int c; // for getopt
char *o_arg = NULL; // used to save path of output file if specified
char *file0,*file1; //buffer to save files in 
bool casesensitive = true; // is set false if -i occurs

while( (c = getopt(argc, argv, "io:")) != -1){
    switch(c){
        case 'o':
            o_arg = optarg;
            break;
        case 'i': 
            casesensitive = false;
            break;
        case '?': 
            usage();
            break;
        default:
            assert(0);
    }
}

if((argc - optind) != 2){
    printf("Number of positional Arguments must be 2!\n");
    usage();
}
if((argc - optind) == 2){
    file0 = argv[optind];
    file1 = argv[optind +1];
    compare(file0,file1,casesensitive,o_arg);
}

return 0;
}
